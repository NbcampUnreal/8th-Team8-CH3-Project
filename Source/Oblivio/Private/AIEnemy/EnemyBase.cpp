#include "AIEnemy/EnemyBase.h"
#include "AIEnemy/EnemyAIController.h"
#include "AIController.h"
#include "OblivioGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

// =============================================================================
// AEnemyBase 구현 요약
// BeginPlay: 체력·이속 초기화 후 어그로/패트롤에 맞춰 첫 상태
// Tick    : 생존·빛CC 통과 시 FSM 갱신 + 상태별 MoveTo / 공격
// UpdateState: 어그로 있으면 Chase/Attack, 없으면 Investigate→Search→Patrol→Idle 우선순위
// CC: Slow/Stun 은 RefreshWalkSpeedFromSources 에서 빛 배율과 곱해 적용
// =============================================================================

// AIController 클래스 지정, 이동은 폰 회전 맞춤
AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;

	AIControllerClass = AEnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
}

// 생존·체력·첫 FSM. 패트롤 포인트가 있으면 플레이어 없을 때 Patrol로 시작 가능.
void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
	RefreshWalkSpeedFromSources();
	FindDefaultTarget();

	if (HasValidAggroTarget())
	{
		LastKnownTargetLocation = TargetActor->GetActorLocation();
		bHadAggroLastTick = true;
		SetEnemyState(IsTargetInAttackRange() ? EEnemyAIState::Attack : EEnemyAIState::Chase);
	}
	else if (!PatrolPoints.IsEmpty())
	{
		bHadAggroLastTick = false;
		SetEnemyState(EEnemyAIState::Patrol);
	}
	else
	{
		bHadAggroLastTick = false;
		SetEnemyState(EEnemyAIState::Idle);
	}

	// 스폰 시 기본 상태가 이미 Idle이면 SetEnemyState가 no-op → 배회 쿨 초기화
	if (EnemyState == EEnemyAIState::Idle && bEnableIdleWander)
	{
		IdleWanderRetargetCooldown = FMath::FRandRange(0.5f, 2.0f);
	}
}

// 빛 정지 중엔 노출 감쇠 없음(기존 동작). CC 경직 중엔 감쇠는 수행.
void AEnemyBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!IsAlive())
	{
		return;
	}

	if (!IsValid(TargetActor))
	{
		FindDefaultTarget();
	}

	// 경직/빛정지 중에도 FSM은 갱신(Chase 등으로 전환). 이동·공격 스위치만 아래에서 생략.
	UpdateState();

	if (!bIsLightFrozen)
	{
		UpdateLightExposure(DeltaSeconds);
	}

	DrawAggroDebug();

	if (bIsLightFrozen || bCCStunned)
	{
		return;
	}

	switch (EnemyState)
	{
	case EEnemyAIState::Idle:
		UpdateIdle(DeltaSeconds);
		break;
	case EEnemyAIState::Chase:
		UpdateChase();
		break;
	case EEnemyAIState::Attack:
		UpdateAttack();
		break;
	case EEnemyAIState::Patrol:
		UpdatePatrol(DeltaSeconds);
		break;
	case EEnemyAIState::Investigate:
		UpdateInvestigate(DeltaSeconds);
		break;
	case EEnemyAIState::Search:
		UpdateSearch(DeltaSeconds);
		break;
	case EEnemyAIState::Dead:
	default:
		break;
	}
}

EEnemyCCState AEnemyBase::GetCrowdControlState() const
{
	if (!IsAlive())
	{
		return EEnemyCCState::None;
	}
	if (bCCStunned || bIsLightFrozen)
	{
		return EEnemyCCState::Stunned;
	}
	if (bCCSlowActive)
	{
		return EEnemyCCState::Slowed;
	}
	const float LightMult = ComputeLightSpeedMultiplier();
	if (LightMult < 1.0f - KINDA_SMALL_NUMBER)
	{
		return EEnemyCCState::Slowed;
	}
	return EEnemyCCState::None;
}

void AEnemyBase::ApplyCCSlow(float SpeedMultiplier, float Duration)
{
	if (!IsAlive())
	{
		return;
	}

	const float Clamped = FMath::Clamp(SpeedMultiplier, 0.0f, 1.0f);
	bCCSlowActive = true;
	CCSlowSpeedMultiplier = FMath::Min(CCSlowSpeedMultiplier, Clamped);

	if (Duration > KINDA_SMALL_NUMBER)
	{
		float NewDuration = Duration;
		if (UWorld* World = GetWorld())
		{
			const float Remaining = World->GetTimerManager().GetTimerRemaining(CCSlowTimerHandle);
			NewDuration = FMath::Max(Remaining, Duration);
		}
		GetWorldTimerManager().SetTimer(CCSlowTimerHandle, this, &AEnemyBase::OnCCSlowExpired, NewDuration, false);
	}
	else
	{
		GetWorldTimerManager().ClearTimer(CCSlowTimerHandle);
	}

	RefreshWalkSpeedFromSources();
}

void AEnemyBase::ApplyCCStun(float Duration)
{
	if (!IsAlive())
	{
		return;
	}

	bCCStunned = true;
	StopEnemyMovement();
	RefreshWalkSpeedFromSources();

	if (Duration > KINDA_SMALL_NUMBER)
	{
		float NewDuration = Duration;
		if (UWorld* World = GetWorld())
		{
			const float Remaining = World->GetTimerManager().GetTimerRemaining(CCStunTimerHandle);
			NewDuration = FMath::Max(Remaining, Duration);
		}
		GetWorldTimerManager().SetTimer(CCStunTimerHandle, this, &AEnemyBase::OnCCStunExpired, NewDuration, false);
	}
	else
	{
		GetWorldTimerManager().ClearTimer(CCStunTimerHandle);
	}
}

void AEnemyBase::ClearCCSlow()
{
	bCCSlowActive = false;
	CCSlowSpeedMultiplier = 1.0f;
	GetWorldTimerManager().ClearTimer(CCSlowTimerHandle);
	if (IsAlive())
	{
		RefreshWalkSpeedFromSources();
	}
}

void AEnemyBase::ClearCCStun()
{
	bCCStunned = false;
	GetWorldTimerManager().ClearTimer(CCStunTimerHandle);
	if (IsAlive())
	{
		RefreshWalkSpeedFromSources();
		UpdateState();
	}
}

void AEnemyBase::OnCCSlowExpired()
{
	bCCSlowActive = false;
	CCSlowSpeedMultiplier = 1.0f;
	if (IsAlive())
	{
		RefreshWalkSpeedFromSources();
	}
}

void AEnemyBase::OnCCStunExpired()
{
	bCCStunned = false;
	if (IsAlive())
	{
		RefreshWalkSpeedFromSources();
		UpdateState();
	}
}

// 체력 차감·피격 델리게이트·0이하면 Die. 반환값은 실제 적용된 데미지
float AEnemyBase::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float AppliedDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (!IsAlive() || AppliedDamage <= 0.0f)
	{
		return 0.0f;
	}

	CurrentHealth = FMath::Clamp(CurrentHealth - AppliedDamage, 0.0f, MaxHealth);

	UE_LOG(LogTemp, Verbose, TEXT("%s took %.1f damage. Health: %.1f / %.1f"),
		*GetNameSafe(this), AppliedDamage, CurrentHealth, MaxHealth);

	OnEnemyDamaged.Broadcast(AppliedDamage, CurrentHealth, MaxHealth);

	if (CurrentHealth <= 0.0f)
	{
		Die();
	}

	return AppliedDamage;
}

// 손전등 등: 노출 누적 → 사망 임계/둔화/정지 → 타이머로 일정 시간 후 이동 복구
void AEnemyBase::OnLightHit(float Intensity, float Duration)
{
	if (!IsAlive())
	{
		return;
	}

	const float ClampedDuration = FMath::Max(0.0f, Duration);
	const float ClampedIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
	LightExposure = FMath::Max(0.0f, LightExposure + ClampedIntensity * ClampedDuration);

	if (LightDeathExposureThreshold > 0.0f && LightExposure >= LightDeathExposureThreshold)
	{
		Die();
		return;
	}

	float SpeedMultiplier = 1.0f;
	if (LightExposure >= LightFreezeExposureThreshold)
	{
		SpeedMultiplier = LightFreezeSpeedMultiplier;
	}
	else if (LightExposure >= LightSlowExposureThreshold)
	{
		SpeedMultiplier = LightSlowSpeedMultiplier;
	}

	bIsLightFrozen = ClampedDuration > 0.0f && SpeedMultiplier <= KINDA_SMALL_NUMBER;
	RefreshWalkSpeedFromSources();

	if (bIsLightFrozen)
	{
		StopEnemyMovement();
	}

	if (ClampedDuration > 0.0f)
	{
		GetWorldTimerManager().ClearTimer(LightFreezeTimerHandle);
		GetWorldTimerManager().SetTimer(LightFreezeTimerHandle, this, &AEnemyBase::RestoreMovementAfterLight, ClampedDuration, false);
	}
}

// 플래시뱅: 인자 없으면 DefaultFlashbangDamage로 일반 TakeDamage 경로 태움
void AEnemyBase::OnFlashbangHit(float DamageAmount)
{
	if (!IsAlive())
	{
		return;
	}

	const float DamageToApply = DamageAmount > 0.0f ? DamageAmount : DefaultFlashbangDamage;
	UGameplayStatics::ApplyDamage(this, DamageToApply, nullptr, nullptr, UDamageType::StaticClass());
}

// 수동으로 추적 대상 교체 후 FSM 즉시 갱신
void AEnemyBase::SetTargetActor(AActor* NewTarget)
{
	TargetActor = NewTarget;
	UpdateState();
}

// 어그로 대상이 없을 때만: 소음 등 위치로 Investigate 큐 적재
void AEnemyBase::ReportStimulus(FVector WorldLocation)
{
	if (!IsAlive())
	{
		return;
	}
	if (HasValidAggroTarget())
	{
		return;
	}

	PendingInvestigateLocation = WorldLocation;
	bHasPendingInvestigate = true;
	InvestigateTimerRemaining = InvestigateStimulusTimeout;
	UpdateState();
}

// 월드 플레이어 0번 폰( Character 아님 포함). 없으면 타겟 없음 → 어그로 실패 가능
void AEnemyBase::FindDefaultTarget()
{
	TargetActor = UGameplayStatics::GetPlayerPawn(this, 0);
}

// 유효 폰 + AggroRadius(0이면 거리 무시). 수평 전용이면 Z 무시.
bool AEnemyBase::HasValidAggroTarget() const
{
	if (!IsValid(TargetActor))
	{
		return false;
	}
	if (AggroRadius <= 0.0f)
	{
		return true;
	}

	const FVector A = GetActorLocation();
	const FVector B = TargetActor->GetActorLocation();
	const float DistSq = bAggroUseHorizontalDistance
		? FVector::DistSquared(FVector(A.X, A.Y, 0.0f), FVector(B.X, B.Y, 0.0f))
		: FVector::DistSquared(A, B);

	return DistSq <= FMath::Square(AggroRadius);
}

// 어그로 → Chase/Attack, 없으면 방금 놓침이면 Search 타이머, 그다음 자극/탐색/패트롤/Idle
void AEnemyBase::UpdateState()
{
	if (!IsAlive())
	{
		SetEnemyState(EEnemyAIState::Dead);
		return;
	}

	const bool bAggro = HasValidAggroTarget();

	if (bAggro)
	{
		LastKnownTargetLocation = TargetActor->GetActorLocation();
		SearchTimeRemaining = 0.0f;
		SearchRetargetCooldown = 0.0f;
		bHasPendingInvestigate = false;
		SetEnemyState(IsTargetInAttackRange() ? EEnemyAIState::Attack : EEnemyAIState::Chase);
		bHadAggroLastTick = true;
		return;
	}

	if (bHadAggroLastTick && SearchPhaseDuration > 0.0f)
	{
		SearchTimeRemaining = SearchPhaseDuration;
		SearchAnchor = LastKnownTargetLocation;
		SearchRetargetCooldown = 0.0f;
	}
	bHadAggroLastTick = false;

	if (bHasPendingInvestigate)
	{
		SetEnemyState(EEnemyAIState::Investigate);
		return;
	}

	if (SearchTimeRemaining > 0.0f)
	{
		SetEnemyState(EEnemyAIState::Search);
		return;
	}

	if (!PatrolPoints.IsEmpty())
	{
		SetEnemyState(EEnemyAIState::Patrol);
		return;
	}

	SetEnemyState(EEnemyAIState::Idle);
}

// NavMesh MoveToActor로 플레이어 접근
void AEnemyBase::UpdateChase()
{
	AAIController* EnemyController = Cast<AAIController>(GetController());
	if (!EnemyController || !IsValid(TargetActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s cannot chase: missing controller or target"), *GetNameSafe(this));
		return;
	}

	// Chase 수용이 AttackRange에 너무 가깝거나 크면 미세하게 밖에서 멈춰 Attack 미전환 → AttackRange - ChaseProximityBuffer 로 상한.
	const float MaxAcceptanceForAttack = FMath::Max(1.0f, AttackRange - ChaseProximityBuffer);
	const float EffectiveChaseAcceptance = FMath::Min(ChaseAcceptanceRadius, MaxAcceptanceForAttack);

	const EPathFollowingRequestResult::Type MoveResult = EnemyController->MoveToActor(TargetActor, EffectiveChaseAcceptance);
	if (MoveResult == EPathFollowingRequestResult::Failed)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s failed to request path to %s"), *GetNameSafe(this), *GetNameSafe(TargetActor));
	}
}

// 정지 후 쿨다운 맞으면 PerformAttack (BP 오버라이드 가능)
void AEnemyBase::UpdateAttack()
{
	StopEnemyMovement();

	const float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	if (CurrentTime - LastAttackTime < AttackCooldown)
	{
		return;
	}

	LastAttackTime = CurrentTime;
	PerformAttack(TargetActor);
}

// 기본 근접: 범위 내일 때만 대상에게 ApplyDamage
void AEnemyBase::PerformAttack_Implementation(AActor* Target)
{
	if (!IsValid(Target) || !IsTargetInAttackRange())
	{
		return;
	}

	UGameplayStatics::ApplyDamage(Target, AttackDamage, GetController(), this, UDamageType::StaticClass());
}

// PatrolPoints 순서대로 도착 반경 안 들어오면 다음 지점
void AEnemyBase::UpdatePatrol(float DeltaSeconds)
{
	(void)DeltaSeconds;

	if (PatrolPoints.IsEmpty())
	{
		return;
	}

	AAIController* AI = Cast<AAIController>(GetController());
	if (!AI)
	{
		return;
	}

	for (int32 Guard = 0; Guard < PatrolPoints.Num(); ++Guard)
	{
		AActor* const Point = PatrolPoints[CurrentPatrolIndex];
		if (IsValid(Point))
		{
			if (FVector::DistSquared(GetActorLocation(), Point->GetActorLocation()) <= FMath::Square(PatrolAcceptanceRadius))
			{
				CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
				continue;
			}

			const EPathFollowingRequestResult::Type MoveResult = AI->MoveToActor(Point, PatrolAcceptanceRadius);
			if (MoveResult == EPathFollowingRequestResult::Failed)
			{
				UE_LOG(LogTemp, Verbose, TEXT("%s patrol MoveToActor failed"), *GetNameSafe(this));
			}
			return;
		}

		CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
	}
}

// 어그로 밖·Patrol 없음: 주변 Nav 랜덤 지점으로 배회
void AEnemyBase::UpdateIdle(float DeltaSeconds)
{
	(void)DeltaSeconds;

	if (!bEnableIdleWander || HasValidAggroTarget())
	{
		StopEnemyMovement();
		return;
	}

	AAIController* AI = Cast<AAIController>(GetController());
	if (!AI)
	{
		return;
	}

	IdleWanderRetargetCooldown -= DeltaSeconds;
	if (IdleWanderRetargetCooldown > 0.0f)
	{
		return;
	}

	IdleWanderRetargetCooldown = IdleWanderRetargetInterval;

	UWorld* World = GetWorld();
	UNavigationSystemV1* NavSys = World ? FNavigationSystem::GetCurrent<UNavigationSystemV1>(World) : nullptr;
	if (!NavSys)
	{
		IdleWanderRetargetCooldown = FMath::Min(IdleWanderRetargetCooldown, 1.0f);
		return;
	}

	const FVector Origin = GetActorLocation();
	const FVector2D Offset2D = FMath::RandPointInCircle(IdleWanderRadius);
	const FVector Candidate = Origin + FVector(Offset2D.X, Offset2D.Y, 0.0f);

	FNavLocation Projected;
	if (NavSys->ProjectPointToNavigation(Candidate, Projected, FVector(250.0f, 250.0f, 300.0f)))
	{
		const EPathFollowingRequestResult::Type MoveResult = AI->MoveToLocation(Projected.Location, IdleWanderAcceptanceRadius);
		if (MoveResult == EPathFollowingRequestResult::Failed)
		{
			UE_LOG(LogTemp, Verbose, TEXT("%s idle wander MoveToLocation failed"), *GetNameSafe(this));
			IdleWanderRetargetCooldown = FMath::Min(1.0f, IdleWanderRetargetInterval * 0.25f);
		}
	}
	else
	{
		IdleWanderRetargetCooldown = FMath::Min(1.0f, IdleWanderRetargetInterval * 0.25f);
	}
}

// 자극 지점까지 MoveToLocation, 도착/타임아웃 시 큐 해제 후 UpdateState
void AEnemyBase::UpdateInvestigate(float DeltaSeconds)
{
	AAIController* AI = Cast<AAIController>(GetController());
	if (!AI)
	{
		return;
	}

	const float DistSq = FVector::DistSquared(GetActorLocation(), PendingInvestigateLocation);
	if (DistSq <= FMath::Square(InvestigateAcceptanceRadius))
	{
		bHasPendingInvestigate = false;
		InvestigateTimerRemaining = 0.0f;
		UpdateState();
		return;
	}

	InvestigateTimerRemaining -= DeltaSeconds;
	if (InvestigateTimerRemaining <= 0.0f)
	{
		bHasPendingInvestigate = false;
		UpdateState();
		return;
	}

	const EPathFollowingRequestResult::Type MoveResult = AI->MoveToLocation(PendingInvestigateLocation, InvestigateAcceptanceRadius);
	if (MoveResult == EPathFollowingRequestResult::Failed)
	{
		UE_LOG(LogTemp, Verbose, TEXT("%s investigate MoveToLocation failed"), *GetNameSafe(this));
	}
}

// 어그로 상실 후: SearchAnchor 주변 랜덤 점을 주기적으로 재요청
void AEnemyBase::UpdateSearch(float DeltaSeconds)
{
	SearchTimeRemaining -= DeltaSeconds;
	if (SearchTimeRemaining <= 0.0f)
	{
		SearchTimeRemaining = 0.0f;
		UpdateState();
		return;
	}

	SearchRetargetCooldown -= DeltaSeconds;
	if (SearchRetargetCooldown > 0.0f)
	{
		return;
	}

	SearchRetargetCooldown = SearchRetargetInterval;

	AAIController* AI = Cast<AAIController>(GetController());
	if (!AI)
	{
		return;
	}

	const FVector2D Offset2D = FMath::RandPointInCircle(SearchRadius);
	const FVector Dest = SearchAnchor + FVector(Offset2D.X, Offset2D.Y, 0.0f);
	const EPathFollowingRequestResult::Type MoveResult = AI->MoveToLocation(Dest, PatrolAcceptanceRadius);
	if (MoveResult == EPathFollowingRequestResult::Failed)
	{
		UE_LOG(LogTemp, Verbose, TEXT("%s search MoveToLocation failed"), *GetNameSafe(this));
	}
}

// Dead 상태·충돌 끔·이벤트·시체 잔존 시간 후 파괴
void AEnemyBase::Die()
{
	if (EnemyState == EEnemyAIState::Dead)
	{
		return;
	}

	CurrentHealth = 0.0f;
	SetEnemyState(EEnemyAIState::Dead);
	StopEnemyMovement();

	GetWorldTimerManager().ClearTimer(LightFreezeTimerHandle);
	GetWorldTimerManager().ClearTimer(CCSlowTimerHandle);
	GetWorldTimerManager().ClearTimer(CCStunTimerHandle);
	bIsLightFrozen = false;
	bCCSlowActive = false;
	bCCStunned = false;
	CCSlowSpeedMultiplier = 1.0f;

	GetCharacterMovement()->DisableMovement();
	SetActorEnableCollision(false);
	OnEnemyDied.Broadcast(this);

	SetLifeSpan(FMath::Max(0.0f, CorpseLifeSpan));

	//추가: 인스턴스에서 킬카운트를 기록
	if (AOblivioGameMode* GM = Cast<AOblivioGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
	{
		GM->AddMonsterKill();
	}
}

// 동일 상태면 무시(로그 스팸 방지). Verbose 로그만 출력
void AEnemyBase::SetEnemyState(EEnemyAIState NewState)
{
	if (EnemyState == NewState)
	{
		return;
	}

	const EEnemyAIState OldState = EnemyState;
	EnemyState = NewState;
	if (NewState == EEnemyAIState::Idle && bEnableIdleWander)
	{
		IdleWanderRetargetCooldown = FMath::FRandRange(0.3f, 1.5f);
	}
	NotifyEnemyStateChanged(OldState, NewState);
	UE_LOG(LogTemp, Verbose, TEXT("%s state changed to %s"), *GetNameSafe(this), *UEnum::GetValueAsString(EnemyState));
}

// AttackRange 제곱 거리로 근접 판정
bool AEnemyBase::IsTargetInAttackRange() const
{
	return IsValid(TargetActor) && FVector::DistSquared(GetActorLocation(), TargetActor->GetActorLocation()) <= FMath::Square(AttackRange);
}

// AI 이동 요청 취소(공격/정지 시)
void AEnemyBase::StopEnemyMovement()
{
	if (AAIController* EnemyController = Cast<AAIController>(GetController()))
	{
		EnemyController->StopMovement();
	}
}

// 매 틱 LightExposure 선형 감쇠 + 이속 갱신(FSM 전환 직후에도 Chase/Wander 반영)
void AEnemyBase::UpdateLightExposure(float DeltaSeconds)
{
	if (LightExposure > 0.0f && LightExposureDecayPerSecond > 0.0f)
	{
		LightExposure = FMath::Max(0.0f, LightExposure - LightExposureDecayPerSecond * DeltaSeconds);
	}
	if (IsAlive())
	{
		RefreshWalkSpeedFromSources();
	}
}

float AEnemyBase::ComputeLightSpeedMultiplier() const
{
	if (LightDeathExposureThreshold > 0.0f && LightExposure >= LightDeathExposureThreshold)
	{
		return 0.0f;
	}
	if (LightExposure >= LightFreezeExposureThreshold)
	{
		return LightFreezeSpeedMultiplier;
	}
	if (LightExposure >= LightSlowExposureThreshold)
	{
		return LightSlowSpeedMultiplier;
	}
	return 1.0f;
}

void AEnemyBase::RefreshWalkSpeedFromSources()
{
	if (!IsAlive() || !GetCharacterMovement())
	{
		return;
	}

	const bool bImmobilized = bIsLightFrozen || bCCStunned;
	if (bImmobilized)
	{
		GetCharacterMovement()->MaxWalkSpeed = 0.0f;
		return;
	}

	const float LightMult = ComputeLightSpeedMultiplier();
	const float CCSlowMult = bCCSlowActive ? CCSlowSpeedMultiplier : 1.0f;
	const float BaseSpeed = GetLocomotionBaseSpeed();
	GetCharacterMovement()->MaxWalkSpeed = BaseSpeed * LightMult * CCSlowMult;
}

float AEnemyBase::GetLocomotionBaseSpeed() const
{
	const bool bCombatLocomotion =
		EnemyState == EEnemyAIState::Chase ||
		EnemyState == EEnemyAIState::Attack;
	if (bCombatLocomotion && ChaseMoveSpeed > KINDA_SMALL_NUMBER)
	{
		return ChaseMoveSpeed;
	}
	return MoveSpeed;
}

void AEnemyBase::DrawAggroDebug()
{
#if UE_BUILD_SHIPPING
	return;
#else
	if (!bDebugDrawAggroRadius || AggroRadius <= 0.0f || !GetWorld())
	{
		return;
	}

	if (!IsValid(TargetActor))
	{
		FindDefaultTarget();
	}

	const bool bPausedAi = bIsLightFrozen || bCCStunned;
	const bool bInRange = HasValidAggroTarget();

	FColor Color;
	if (bPausedAi && bInRange)
	{
		Color = FColor::Yellow;
	}
	else if (bPausedAi)
	{
		Color = FColor::Cyan;
	}
	else if (bInRange)
	{
		Color = FColor::Green;
	}
	else
	{
		Color = FColor::Orange;
	}

	DrawDebugSphere(GetWorld(), GetActorLocation(), AggroRadius, 28, Color, false, 0.0f, 0, 1.5f);

	const FVector TextPos = GetActorLocation() + FVector(0.f, 0.f, AggroRadius * 0.2f + 50.f);
	const FString Line = FString::Printf(
		TEXT("%s | Aggro:%s%s"),
		*UEnum::GetValueAsString(EnemyState),
		bInRange ? TEXT("IN") : TEXT("OUT"),
		bPausedAi ? TEXT(" | Move:PAUSED") : TEXT(""));
	DrawDebugString(GetWorld(), TextPos, Line, nullptr, Color, 0.0f, true, 1.05f);
#endif
}

// OnLightHit 타이머 만료: 빛 정지 해제 후 빛·CC 배율 재적용 및 FSM 갱신
void AEnemyBase::RestoreMovementAfterLight()
{
	if (!IsAlive())
	{
		return;
	}

	bIsLightFrozen = false;
	RefreshWalkSpeedFromSources();
	UpdateState();
}

void AEnemyBase::SetEnemySoundVolumeMultiplier(float NewMultiplier)
{
	EnemySoundVolumeMultiplier = FMath::Clamp(NewMultiplier, 0.0f, 4.0f);
	ApplyEnemySoundVolumes();
}

void AEnemyBase::ApplyEnemySoundVolumes()
{
}
