#include "AIEnemy/BasicEnemy.h"

ABasicEnemy::ABasicEnemy()
{
	// 기본 몬스터: 기획용 기본 수치. BP Class Defaults에서 메시·애니·콜리전과 함께 조정.
	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;
	MoveSpeed = 350.0f;
	AttackDamage = 10.0f;
	AttackRange = 150.0f;
	AttackCooldown = 1.0f;
	ChaseAcceptanceRadius = 80.0f;
	CorpseLifeSpan = 3.0f;
}
