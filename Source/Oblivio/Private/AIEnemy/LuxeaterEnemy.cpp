#include "AIEnemy/LuxeaterEnemy.h"

#include "AIController.h"

ALuxeaterEnemy::ALuxeaterEnemy()
{
	// 6F boss baseline. Combat results are delegated; these values describe AI pressure.
	MaxHealth = 600.0f;
	CurrentHealth = MaxHealth;
	MoveSpeed = 180.0f;
	ChaseMoveSpeed = 230.0f;
	AttackDamage = 0.0f;
	AttackRange = 220.0f;
	AttackCooldown = 1.2f;
	ChaseAcceptanceRadius = 65.0f;
	ChaseProximityBuffer = 45.0f;
	AggroRadius = 0.0f;
	bAggroUseHorizontalDistance = true;
	bEnableIdleWander = false;
	bEnableLightTracking = false;
}

void ALuxeaterEnemy::BeginPlay()
{
	BaseMoveSpeed = MoveSpeed;
	BaseChaseMoveSpeed = ChaseMoveSpeed > KINDA_SMALL_NUMBER ? ChaseMoveSpeed : MoveSpeed;
	InitialScale = GetActorScale3D();

	AggroRadius = 0.0f;
	bEnableIdleWander = false;
	bEnableLightTracking = false;

	Super::BeginPlay();
}

void ALuxeaterEnemy::OnLightHit(float Intensity, float Duration)
{
	if (!IsAlive())
	{
		return;
	}

	const float ClampedIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
	const float ClampedDuration = FMath::Max(0.0f, Duration);
	const float AbsorbedAmount = ClampedIntensity * ClampedDuration;

	// EnemyBase only broadcasts light-hit judgement. Luxeater additionally consumes it.
	Super::OnLightHit(ClampedIntensity, ClampedDuration);

	if (AbsorbedAmount <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	AbsorbedLight += AbsorbedAmount;
	OnLightAbsorbed.Broadcast(this, AbsorbedAmount, AbsorbedLight);

	// 체력은 변하지 않음 — 이동속도·크기만 증가.
	ApplyLightEmpowerment();
}

void ALuxeaterEnemy::NotifyBossHealthChanged(float NewCurrentHealth, float NewMaxHealth)
{
	// 전투 모듈 소스 오브 트루스 체력을 여기에 미러링해서 페이즈 판정에만 사용한다.
	MaxHealth = FMath::Max(1.0f, NewMaxHealth);
	CurrentHealth = FMath::Clamp(NewCurrentHealth, 0.0f, MaxHealth);
	UpdateHealthPhase();
}

void ALuxeaterEnemy::NotifyEnemyDamageApplied(float /*AppliedDamage*/)
{
	UpdateHealthPhase();
}

void ALuxeaterEnemy::UpdateChase()
{
	AAIController* const AI = Cast<AAIController>(GetController());
	if (!AI || !IsValid(TargetActor))
	{
		return;
	}

	AI->MoveToActor(TargetActor, FMath::Max(5.0f, AttackRange - ChaseProximityBuffer), false);
}

void ALuxeaterEnemy::UpdateAttack()
{
	// Base UpdateAttack only decides attack timing and calls PerformAttack.
	Super::UpdateAttack();
}

void ALuxeaterEnemy::ApplyLightEmpowerment()
{
	const float SpeedBonus = FMath::Min(MaxLightSpeedBonus, AbsorbedLight * SpeedGainPerLight);
	MoveSpeed = BaseMoveSpeed + SpeedBonus;
	ChaseMoveSpeed = BaseChaseMoveSpeed + SpeedBonus;
	RefreshWalkSpeedFromSources();

	const float ScaleMultiplier = FMath::Min(MaxLightScaleMultiplier, 1.0f + AbsorbedLight * ScaleGainPerLight);
	SetActorScale3D(InitialScale * ScaleMultiplier);
}

void ALuxeaterEnemy::UpdateHealthPhase()
{
	const float HealthPercent = MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f;
	const int32 NewPhase = HealthPercent <= PhaseTwoHealthPercentThreshold ? 2 : 1;
	if (NewPhase == BossPhase)
	{
		return;
	}

	const int32 OldPhase = BossPhase;
	BossPhase = NewPhase;
	OnPhaseChanged.Broadcast(this, OldPhase, BossPhase);
}
