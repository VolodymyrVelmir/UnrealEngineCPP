
#include "rUnit.h"
 
ArUnit::ArUnit()
{
	PrimaryActorTick.bCanEverTick = false;
}
 
void ArUnit::BeginPlay()
{
	Super::BeginPlay();
}

void ArUnit::SetBlockAttack(bool isBlock, bool isOnnCD) {
	isBlockAttack = isBlock;
	if (isBlockAttack && isOnnCD) {
		GetWorld()->GetTimerManager().SetTimer(TimerAttackCD, this, &ArUnit::BlockAttackEnd, AttackCD);
	}
}

void ArUnit::BlockAttackEnd() {
	SetBlockAttack(false, false);
}

void ArUnit::AddHP(int32 ValueAdd) {
	if (GetUnitHP() + ValueAdd <= UnitHPMax) {
		SetUnitHP(GetUnitHP() + ValueAdd);
		if (GetUnitHP() <= 0) { DeathUnit(true); }
		OnUpdatUnitHP.Broadcast();
	}
}

void ArUnit::DeathUnit(bool isStart) {
	ShowDeathEfects(isStart);
	if (isStart && !GetUnitDeath()) {
		OnUnitDeath.Broadcast();
		SetBlockAttack(true, false);
		SetBlockMove(true);
		isDeathUnit=true;
		GetWorld()->GetTimerManager().SetTimer(TimerDeath, this, &ArUnit::DeathUnitEnd, DeathTaimer);
	}
	else {
		Destroy();
	}
}
void ArUnit::DeathUnitEnd() {
	DeathUnit(false);
}

bool ArUnit::MoveLeftOrRight(bool isLeft) {
	if (!GetBlockMove()) {
		if (isLeft != GetMoveLeft()) { SetUnitTurn(isLeft); }
		SetMoveUnit(true);
		FVector NewLocation = GetActorLocation();
		NewLocation.Y += ((GetMoveLeft()) ? -1 : 1) * GetMoveSpeed();
		SetActorLocation(NewLocation, false, nullptr, ETeleportType::None);
 
		return true;
	}
	return false;
}

void ArUnit::SetUnitTurn(bool isLeftTurn) {
	isUnitTurnLeft = isLeftTurn;
	UpdateUnitTurn(isUnitTurnLeft);
}

bool ArUnit::UnitAttack() {
	if (isHaveAmmo() && !GetBlockAttack()) {
		SetBlockAttack(true, true);
		ArBulet* Bulet=GetWorld()->SpawnActor<ArBulet>(GetBuletDammyClass(), GetShootLocation(), GetShootRotation(), FActorSpawnParameters());
		Bulet->SetOwnerUnit(this, GetMoveLeft());
		Bulet->PlayerFrection = PlayerFrection;
		OnStartAttack.Broadcast();
		return true;
	}
	return false;
}
