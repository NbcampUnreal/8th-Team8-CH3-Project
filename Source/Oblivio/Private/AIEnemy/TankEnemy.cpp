#include "AIEnemy/TankEnemy.h"

ATankEnemy::ATankEnemy()
{
	// Basic 대비 대략 3배 체력, ~63% 이속, 데미지+5, 약간 느린 공격
	MaxHealth = 300.0f;
	CurrentHealth = MaxHealth;
	MoveSpeed = 220.0f;
	ChaseMoveSpeed = 280.0f;
	AttackDamage = 15.0f;
	// 이속이 느리고 메시가 크면 150에서 Chase→Attack 미전환이 잦음. 베이직 추격 튜닝 + 근접 거리 소폭↑.
	AttackRange = 200.0f;
	AttackCooldown = 1.25f;
	ChaseAcceptanceRadius = 55.0f;
	ChaseProximityBuffer = 48.0f;
	AggroRadius = 1000.0f;
	CorpseLifeSpan = 3.0f;
}
