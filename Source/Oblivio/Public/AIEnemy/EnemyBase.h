#pragma once

// =============================================================================
// AEnemyBase — 모든 적 캐릭터의 공통 부모.
//
// FSM(의사결정): Idle → Chase / Attack → (어그로 없음 시) Investigate → Search → Patrol → Idle
//   · ReportStimulus: 소리·트리거 등 외부 자극 위치(어그로 없을 때 우선 Investigate)
//   · AggroRadius: UE 단위 cm (1000 ≈ 10m). 0이면 거리 무시 항상 추격. 타겟은 GetPlayerPawn(0)
//
// 빛: OnLightHit으로 노출(LightExposure) 누적 → 임계값에 따라 이동속 둔화/정지, 과다 시 Die
//   · 빛 정지 중(bIsLightFrozen)에는 Tick에서 추격·패트롤 등 미실행(연출/시퀀스와 협업 시 IsLightCrowdFrozen 참고)
//
// CC(기술·아이템 등): EEnemyCCState — Slow(이속 배율), Stun(경직·행동 정지). 빛 이속 배율과 곱함.
//   · Duration<=0 이면 타이머 없이 유지 → ClearCCSlow / ClearCCStun 으로 해제
//
// NavMesh가 있어야 MoveToActor / MoveToLocation 동작. 레벨에 Nav Mesh Bounds 권장.
// 비어그로 Idle: PatrolPoints 없을 때 IdleWander로 주변 배회(옵션).
// =============================================================================

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyBase.generated.h"

class AEnemyBase;

/** 적 행동 상태(FSM). CC(경직 등)는 별도 플래그로 처리 — 상태 이름은 '의도'만 표현. */
UENUM(BlueprintType)
enum class EEnemyAIState : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Chase UMETA(DisplayName = "Chase"),
	Attack UMETA(DisplayName = "Attack"),
	Patrol UMETA(DisplayName = "Patrol"),
	Investigate UMETA(DisplayName = "Investigate"),
	Search UMETA(DisplayName = "Search"),
	Dead UMETA(DisplayName = "Dead")
};

/** 이동 저하·경직 등 CC(FSM과 별개). GetCrowdControlState는 빛 둔화/정지도 함께 반영. */
UENUM(BlueprintType)
enum class EEnemyCCState : uint8
{
	None UMETA(DisplayName = "None"),
	Slowed UMETA(DisplayName = "Slowed"),
	Stunned UMETA(DisplayName = "Stunned"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEnemyDiedSignature, AEnemyBase*, Enemy);
/** 피격 시: (들어온 데미지, 남은 체력, 최대체력) — UI·히트 이펙트용 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FEnemyDamagedSignature, float, DamageAmount, float, CurrentHealth, float, MaxHealth);

UCLASS(Blueprintable)
class OBLIVIO_API AEnemyBase : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemyBase();

	// 매 틱: 빛 노출 감쇠 → 타겟 재탐색 → FSM 갱신 → 현재 상태 실행(이동/공격/패트롤 등)
	virtual void Tick(float DeltaSeconds) override;
	// 엔진 damage 파이프라인. 체력 반영 후 OnEnemyDamaged, 0 이하면 Die
	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable, Category = "Enemy|State")
	EEnemyAIState GetEnemyState() const { return EnemyState; }

	UFUNCTION(BlueprintCallable, Category = "Enemy|State")
	bool IsAlive() const { return EnemyState != EEnemyAIState::Dead && CurrentHealth > 0.0f; }

	UFUNCTION(BlueprintPure, Category = "Enemy|CrowdControl")
	EEnemyCCState GetCrowdControlState() const;

	/** SpeedMultiplier: 1=정상, 0.5=절반 이속. Duration<=0 이면 ClearCCSlow 할 때까지 유지. */
	UFUNCTION(BlueprintCallable, Category = "Enemy|CrowdControl")
	void ApplyCCSlow(float SpeedMultiplier, float Duration = 0.0f);

	/** 경직: AI/추격·공격 중단. Duration<=0 이면 ClearCCStun 할 때까지. */
	UFUNCTION(BlueprintCallable, Category = "Enemy|CrowdControl")
	void ApplyCCStun(float Duration = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "Enemy|CrowdControl")
	void ClearCCSlow();

	UFUNCTION(BlueprintCallable, Category = "Enemy|CrowdControl")
	void ClearCCStun();

	UFUNCTION(BlueprintPure, Category = "Enemy|CrowdControl")
	bool IsCCStunned() const { return bCCStunned; }

	UFUNCTION(BlueprintCallable, Category = "Enemy|Light")
	virtual void OnLightHit(float Intensity, float Duration);

	UFUNCTION(BlueprintCallable, Category = "Enemy|Light")
	virtual void OnFlashbangHit(float DamageAmount = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "Enemy|Target")
	void SetTargetActor(AActor* NewTarget);

	/** 소리·트리거 등 자극 위치. 어그로 대상이 없을 때 Investigate 우선. */
	UFUNCTION(BlueprintCallable, Category = "Enemy|FSM")
	void ReportStimulus(FVector WorldLocation);

	UFUNCTION(BlueprintPure, Category = "Enemy|Stats")
	float GetCurrentHealthForUI() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category = "Enemy|Stats")
	float GetMaxHealthForUI() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category = "Enemy|Stats")
	float GetHealthPercent() const { return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f; }

	/** 적 SFX 볼륨 배율(0=무음, 1=기본). BP Class Defaults 또는 옵션에서 SetEnemySoundVolumeMultiplier. */
	UFUNCTION(BlueprintCallable, Category = "Enemy|Audio")
	void SetEnemySoundVolumeMultiplier(float NewMultiplier);

	UFUNCTION(BlueprintPure, Category = "Enemy|Audio")
	float GetEnemySoundVolumeMultiplier() const { return EnemySoundVolumeMultiplier; }

	UPROPERTY(BlueprintAssignable, Category = "Enemy|Events")
	FEnemyDiedSignature OnEnemyDied;

	UPROPERTY(BlueprintAssignable, Category = "Enemy|Events")
	FEnemyDamagedSignature OnEnemyDamaged;

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintPure, Category = "Enemy|CrowdControl")
	bool IsLightCrowdFrozen() const { return bIsLightFrozen; }

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Stats", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Stats")
	float CurrentHealth = 100.0f;

	/** 배회·Patrol·Search·Investigate 등 비추격 이동 기준 이속(cm/s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Stats", meta = (ClampMin = "0.0"))
	float MoveSpeed = 350.0f;

	/** Chase·Attack 시 이속. 0이면 MoveSpeed와 동일. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Stats", meta = (ClampMin = "0.0"))
	float ChaseMoveSpeed = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Combat", meta = (ClampMin = "0.0"))
	float AttackDamage = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Combat", meta = (ClampMin = "1.0"))
	float AttackRange = 150.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Combat", meta = (ClampMin = "0.1"))
	float AttackCooldown = 1.0f;

	/** MoveToActor 도착 판정. AttackRange보다 크면 추격이 먼저 멈춰 Chase→Attack 전환이 안 될 수 있음(UpdateChase에서 자동으로 AttackRange보다 안쪽으로 제한). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Navigation", meta = (ClampMin = "1.0"))
	float ChaseAcceptanceRadius = 80.0f;

	/** 추격 수용 반경 상한 = AttackRange - 이 값(cm). 커질수록 더 가까이 붙인 뒤 Chase 종료 → 공격 전환 안정. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Navigation", meta = (ClampMin = "1.0", ClampMax = "500.0"))
	float ChaseProximityBuffer = 40.0f;

	/** 0이면 플레이어가 있으면 항상 어그로. 0보다 크면 이 거리(cm) 밖은 추격 해제. BP·레벨 액터에서 조정 가능. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|FSM", meta = (ClampMin = "0.0"))
	float AggroRadius = 0.0f;

	/** true면 어그로 거리를 XY(수평)만으로 계산. 층 높이 차로 안 닿는 것처럼 보일 때 사용. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|FSM")
	bool bAggroUseHorizontalDistance = true;

	/** PIE/게임에서 어그로 반경 구 디버그(녹=플레이어 인식, 주황=밖). Shipping 빌드에서는 무시. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Debug")
	bool bDebugDrawAggroRadius = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|FSM", meta = (ClampMin = "1.0"))
	float PatrolAcceptanceRadius = 80.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|FSM", meta = (ClampMin = "1.0"))
	float InvestigateAcceptanceRadius = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|FSM", meta = (ClampMin = "0.0"))
	float InvestigateStimulusTimeout = 12.0f;

	/** 어그로를 잃은 뒤 탐색 시간(초). 0이면 Search 비활성. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|FSM", meta = (ClampMin = "0.0"))
	float SearchPhaseDuration = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|FSM", meta = (ClampMin = "1.0"))
	float SearchRadius = 450.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|FSM", meta = (ClampMin = "0.1"))
	float SearchRetargetInterval = 1.5f;

	/** 순찰 지점(빈 액터 등 배치 후 끌어다 놓기). 비어 있으면 Patrol 상태 미사용. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|FSM")
	TArray<TObjectPtr<AActor>> PatrolPoints;

	/** 어그로 밖·Idle이고 Patrol 지점이 없을 때 주변 Nav로 배회. 끄면 제자리 정지(기존 동작). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Idle")
	bool bEnableIdleWander = true;

	/** Idle 배회: 스폰 위치 기준 수평 반경(cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Idle", meta = (ClampMin = "50.0"))
	float IdleWanderRadius = 450.0f;

	/** 새 목표까지 대기 시간(초). 도착 전이면 경로 유지. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Idle", meta = (ClampMin = "0.5"))
	float IdleWanderRetargetInterval = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Idle", meta = (ClampMin = "1.0"))
	float IdleWanderAcceptanceRadius = 64.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Light", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LightSlowSpeedMultiplier = 0.4f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Light", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LightFreezeSpeedMultiplier = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Light", meta = (ClampMin = "0.0"))
	float DefaultFlashbangDamage = 9999.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Light", meta = (ClampMin = "0.0"))
	float LightSlowExposureThreshold = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Light", meta = (ClampMin = "0.0"))
	float LightFreezeExposureThreshold = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Light", meta = (ClampMin = "0.0"))
	float LightDeathExposureThreshold = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Light", meta = (ClampMin = "0.0"))
	float LightExposureDecayPerSecond = 0.35f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Death", meta = (ClampMin = "0.0"))
	float CorpseLifeSpan = 3.0f;

	/** 리얼·루프 등 적 오디오 총괄 배율. 파생 클래스는 ApplyEnemySoundVolumes에서 컴포넌트에 반영. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Audio", meta = (ClampMin = "0.0", ClampMax = "4.0"))
	float EnemySoundVolumeMultiplier = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|State")
	EEnemyAIState EnemyState = EEnemyAIState::Idle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Target")
	TObjectPtr<AActor> TargetActor;

	/** 근접 판정 시 BP에서 오버라이드 가능. 기본은 ApplyDamage만 수행 */
	UFUNCTION(BlueprintNativeEvent, Category = "Enemy|Combat")
	void PerformAttack(AActor* Target);
	virtual void PerformAttack_Implementation(AActor* Target);

	/** 기본 타겟: 월드 플레이어 0번 캐릭터 */
	virtual void FindDefaultTarget();
	/** FSM 전이만 담당(어그로·자극·패트롤 여부). 실제 이동은 Tick 스위치에서 */
	virtual void UpdateState();
	virtual void UpdateChase();
	virtual void UpdateAttack();
	virtual void UpdatePatrol(float DeltaSeconds);
	virtual void UpdateInvestigate(float DeltaSeconds);
	virtual void UpdateSearch(float DeltaSeconds);
	virtual void UpdateIdle(float DeltaSeconds);
	virtual void Die();

	void SetEnemyState(EEnemyAIState NewState);

	/** SetEnemyState에서 실제로 바뀐 직후 호출(Old→New). 기본 빈 구현. */
	virtual void NotifyEnemyStateChanged(EEnemyAIState OldState, EEnemyAIState NewState) {}
	bool IsTargetInAttackRange() const;
	bool HasValidAggroTarget() const;
	void StopEnemyMovement();
	void UpdateLightExposure(float DeltaSeconds);
	void RestoreMovementAfterLight();

	void RefreshWalkSpeedFromSources();
	/** Chase·Attack vs 그 외 이동 기준 이속. 파생 클래스에서 절름발이 추격자 등 이단 속도용 오버라이드. */
	virtual float GetLocomotionBaseSpeed() const;
	float ComputeLightSpeedMultiplier() const;
	void OnCCSlowExpired();
	void OnCCStunExpired();
	void DrawAggroDebug();

	/** SetEnemySoundVolumeMultiplier 이후 호출 — 스토커 등 오디오 컴포넌트 동기화용. */
	virtual void ApplyEnemySoundVolumes();

private:
	float LastAttackTime = -BIG_NUMBER;
	/** 손전등 등으로 누적된 빛 노출. 매 틱 감쇠(LightExposureDecayPerSecond) */
	float LightExposure = 0.0f;
	/** 이동속 0에 가깝게 막힌 상태 — Tick 초반에 return */
	bool bIsLightFrozen = false;
	FTimerHandle LightFreezeTimerHandle;

	bool bCCSlowActive = false;
	float CCSlowSpeedMultiplier = 1.0f;
	FTimerHandle CCSlowTimerHandle;

	bool bCCStunned = false;
	FTimerHandle CCStunTimerHandle;

	bool bHadAggroLastTick = false;
	/** 마지막으로 플레이어를 어그로로 본 월드 위치(Search 앵커용) */
	FVector LastKnownTargetLocation = FVector::ZeroVector;
	int32 CurrentPatrolIndex = 0;

	bool bHasPendingInvestigate = false;
	FVector PendingInvestigateLocation = FVector::ZeroVector;
	float InvestigateTimerRemaining = 0.0f;

	float SearchTimeRemaining = 0.0f;
	FVector SearchAnchor = FVector::ZeroVector;
	float SearchRetargetCooldown = 0.0f;

	float IdleWanderRetargetCooldown = 0.0f;
};
