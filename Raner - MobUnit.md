#pragma once

#include "CoreMinimal.h"
#include "rUnit.h"

#include "rMobUnit.generated.h"

UCLASS()
class RANERA_API ArMobUnit : public ArUnit
{
	GENERATED_BODY()
	
public:	
 
	ArMobUnit();

protected:
 
	virtual void BeginPlay() override;

public:	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agr") FVector ReangeAgre = { 300,50,100 };
	
	bool isHaveTarget(FVector TargetLoc) {
		if (!GetBlockAttack()) {
			FVector MobLoc = GetActorLocation();
			if (
				(((MobLoc.Z - ReangeAgre.Z) <= TargetLoc.Z) && (MobLoc.Z + ReangeAgre.Z) >= TargetLoc.Z) &&
				(((MobLoc.X - ReangeAgre.X) <= TargetLoc.X) && (MobLoc.X + ReangeAgre.X) >= TargetLoc.X) &&
			(
				((!GetMoveLeft() && (MobLoc.Y + ReangeAgre.Y) >= TargetLoc.Y && MobLoc.Y <= TargetLoc.Y)
				|| (GetMoveLeft() && (MobLoc.Y - ReangeAgre.Y) <= TargetLoc.Y && MobLoc.Y >= TargetLoc.Y))
				)
			) {
				return true;
			}
		}
		return false;
	}
	void MoveAi() {
		MoveLeftOrRight(GetnitTurn());
	}
};
