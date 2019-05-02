 #include "rVertikalVorldGenerator.h"


ArVertikalVorldGenerator::ArVertikalVorldGenerator()
{
	PrimaryActorTick.bCanEverTick = false;

}

void ArVertikalVorldGenerator::BeginPlay()
{
	Super::BeginPlay();
	StartLoc=GetActorLocation();
	Random.GenerateNewSeed();
}

 

TSubclassOf<ArPlatform> ArVertikalVorldGenerator::GetRandomPlatformClass() {
	FSpawnLVLinfo&  Info = LVLsInfo[GetGameLVL()];
	int32 SizeRandom = Info.SizePlatformClass;
	int32 RandomIndex = 0;
	if (SizeRandom > 1) {
		RandomIndex = Random.FRandRange(0, SizeRandom);
		if (RandomShance(Info.PlatformClass[RandomIndex].ShanceSpawnPlatform)) {
			return Info.PlatformClass[RandomIndex].PlatformClass;
		}
		else {
			return  GetRandomPlatformClass();
		}
	}
	else {
		return Info.PlatformClass[0].PlatformClass;
	}


}

bool ArVertikalVorldGenerator::GetNoBeckPlatformVertical(int32 IndexLine, int32 IndexLineEnd, int32 ElementIndex) {
	for (int32 index = IndexLine - 1; index >= IndexLineEnd; index--) {
		if (index >= 0) {
			if (Platforms[index].Platform[ElementIndex] != nullptr) {
				return false;
			}
		}
		else { break; }
	}
	return true;
}

bool ArVertikalVorldGenerator::GetNoBeckPlatformLeft(int32 IndexLine, int32 StartIndex, int32 EndIndex) {
	for (int32 index = StartIndex - 1; index >= EndIndex; index--) {
		if (index >= 0) {
			if (Platforms[IndexLine].Platform[index] != nullptr) {
				return false;
			}
		}
		else { break; }
	}
	return true;
}

void ArVertikalVorldGenerator::IsNextLVL(float Distance) {
	LVLFlay += Distance;
	if (LVLFlay >= LVLFlayMax ) {
		ReversFlayDistance();
		if (GetGameLVL()+1 >= ValueLVLs) {
			OnMoveNextLVL.Broadcast(true);
		}else  {
			AddGameLVL();
			RemoveWorldPlatform();
			OnReserPlayerLoc.Broadcast(StartLoc);
			StartCreateWorlds();
			OnMoveNextLVL.Broadcast(false);
		}
		
	}else if(isWorldMinSize && LVLFlay < WorldMinSize){
		ReversFlayDistance();
		OnFlayRegeWorld.Broadcast();
		MoveWorldLimit();
	}
}
 
void ArVertikalVorldGenerator::CreateStartLVL(ArPlayerFlayUnit* PlayerUnit) {
	SetGameLVL(0);
	PlayerFlayUnit = PlayerUnit;
	StartCreateWorlds();
	
	PlayerUnit->OnFlayMove.AddDynamic(this, &ArVertikalVorldGenerator::IsNextLVL);
}

void ArVertikalVorldGenerator::StartCreateWorlds() {
	GetAiManager()->ClearMobMemory();
	Platforms.Empty();
	if (LimitActor) {
		LimitActor->Destroy();
	}
	FSpawnLVLinfo&  Info = LVLsInfo[GetGameLVL()];
	LVLFlayMax = (Info.ValueLineInLVL * LineStepMove)+ LineStepMove;
	FSpawnPlatformInfo LinePlatformInfo;
	FVector SpawnLoc = StartLoc;
	for (int32 iSpawnLine = 0; iSpawnLine < Info.ValueLineInLVL; iSpawnLine++) {
		SpawnLoc.Y = StartLoc.Y;
		SpawnLoc.Y += (StepMove*LinePosition / 2);
		LinePlatformInfo = FSpawnPlatformInfo();
		LinePlatformInfo.Platform.Init(nullptr, LinePosition);
		Platforms.Add(LinePlatformInfo);
		for (int32 iSpawnPlatform = 0; iSpawnPlatform < LinePosition; iSpawnPlatform++) {
			if (!RandomShance(Info.ShanceEmptyPlatform) && GetNoBeckPlatformLeft(iSpawnLine, iSpawnPlatform, iSpawnPlatform - Info.CDSpawnInLine) &&
				GetNoBeckPlatformVertical(iSpawnLine, iSpawnLine - Info.CDSpawnInVertical, iSpawnPlatform)) {
				if (RandomShance(Info.ShanceSpawnPlatform)) {
					Platforms[iSpawnLine].Platform[iSpawnPlatform] =
						GetWorld()->SpawnActor<ArPlatform>(GetRandomPlatformClass(), SpawnLoc, { 0,0,0 }, FActorSpawnParameters());
					Platforms[iSpawnLine].Platform[iSpawnPlatform]->CreateMob(MoveSpeedGameMultyplay);
				}
				else if (RandomShance(Info.ShanceSpawnBox)) {
					Platforms[iSpawnLine].Platform[iSpawnPlatform] =
						GetWorld()->SpawnActor<ArPlatform>(BoxRandomClass, SpawnLoc, { 0,0,0 }, FActorSpawnParameters());
				}
				Platforms[iSpawnLine].ValueSpawnPlatform++;
			}
			SpawnLoc.Y -= StepMove;
		}
		SpawnLoc.Z += LineStepMove;
		
	}
	CreateBox();
	LimitActor=GetWorld()->SpawnActor<AActor>(LimitActorClass, GetLimitLocation(), { 0,0,0 }, FActorSpawnParameters());
	PlayerFlayUnit->RestartStats();
}

void ArVertikalVorldGenerator::CreateBox() {
	TArray<AActor*> AllBoxRandomer;
	TArray<AActor*>  BuferBoxRandomer;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), BoxRandomClass, AllBoxRandomer);
	int32 ValueRandomer = sizeof(AllBoxRandomer) / sizeof(AllBoxRandomer[0]);
	if (ValueRandomer < ValueBoxInLVL) { ValueBoxInLVL = ValueRandomer; }
	if (ValueRandomer != ValueBoxInLVL) {
		for (int32 indexRandom = 0; indexRandom < ValueRandomer; indexRandom++) {
			int32 IndexBox = Random.FRandRange(0, ValueRandomer);
			ValueRandomer--;
			BuferBoxRandomer.Add(AllBoxRandomer[IndexBox]);
			AllBoxRandomer.RemoveAt(IndexBox);
		}
	}
	else {
		BuferBoxRandomer = AllBoxRandomer;
	}
	ArBoxRandomer* BoxRandomer;
	for (AActor* randomer : BuferBoxRandomer) {
		BoxRandomer = Cast<ArBoxRandomer>(randomer);
		if (BoxRandomer) { BoxRandomer->CreateRandomBox(); }
	}
	//DestroyNoUsedBox
	AllBoxRandomer.Empty();
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), BoxRandomClass, AllBoxRandomer);
	for (AActor* platform : AllBoxRandomer) {
		ArPlatform* Platform = Cast<ArPlatform>(platform);
		if (Platform) { Platform->DestroyPlatform(); }
	}
}

void ArVertikalVorldGenerator::RemoveWorldPlatform() {

	TArray<AActor*> AllPlatforms;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ArPlatform::StaticClass(), AllPlatforms);
	for (AActor* platform : AllPlatforms) {
		ArPlatform* Platform = Cast<ArPlatform>(platform);
		if (Platform) { 
	
			Platform->DestroyPlatform(); }
	}


	TArray<AActor* > AllBox;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ArBox::StaticClass(), AllBox);
	for (AActor* box : AllBox) {
		ArBox* Box = Cast<ArBox>(box);
		if (Box) { Box->DestroyBoxEnd(); }
	}
	Platforms.Empty();
}
