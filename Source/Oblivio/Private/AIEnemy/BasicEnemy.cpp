#include "AIEnemy/BasicEnemy.h"

ABasicEnemy::ABasicEnemy()
{
	// 기본 몬스터: 기획용 기본 수치. BP Class Defaults에서 메시·애니·콜리전과 함께 조정.
	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;
	MoveSpeed = 350.0f;
	ChaseMoveSpeed = 420.0f;
	AttackDamage = 10.0f;
	AttackRange = 150.0f;
	AttackCooldown = 1.0f;
	ChaseAcceptanceRadius = 55.0f;
	ChaseProximityBuffer = 48.0f;
	// 0이면 플레이어만 있으면 무한 추격. 베이직은 반경 밖이면 Chase/Attack 해제(패트롤·Idle 등).
	AggroRadius = 1000.0f;
	CorpseLifeSpan = 3.0f;
}
