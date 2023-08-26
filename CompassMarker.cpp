// Fill out your copyright notice in the Description page of Project Settings.


#include "CompassMarker.h"
#include "CompassSubsystem.h"
#include "Kismet/GameplayStatics.h"

void UCompassMarker::SetMarkerPrimitive(FMarkerPrimitive* InPrimitive, FName InMarkerID)
{
	MarkweID = InMarkerID;
 
	PostInitUpdate(*InPrimitive);
}

FVector UCompassMarker::GetTargetLocation()
{ 
	if (FMarkerPrimitive* const Primitive = FindMarkerPrimitive())
	{
		return Primitive->GetTargetLocation();
	}
	return FVector::ZeroVector;
}

bool UCompassMarker::IsActualToMove()
{
	if (FMarkerPrimitive* const Primitive = FindMarkerPrimitive())
	{
		return Primitive->IsValidTarget();
	}
	return false;
}

FMarkerPrimitive* UCompassMarker::FindMarkerPrimitive()
{
	const UGameInstance* const GameInstance = UGameplayStatics::GetGameInstance(GetWorld());
	UCompassSubsystem* const CompassSubsystem = GameInstance->GetSubsystem<UCompassSubsystem>();
	 
	return CompassSubsystem->FindMarkerPrimitive(MarkweID);
}

void UCompassMarker::UpdateMarker()
{
	if (const FMarkerPrimitive* const Primitive = FindMarkerPrimitive())
	{
		PostInitUpdate(*Primitive);
	}
}
 
bool UCompassMarker::IsEnableShadow() 
{ 
	if (const FMarkerPrimitive* const Primitive = FindMarkerPrimitive())
	{
		return Primitive->IsEnableShadow();
	}
	return false;
}

bool UCompassMarker::IsEnableShadowState()
{ 
	if (FMarkerPrimitive* const Primitive = FindMarkerPrimitive())
	{
		return Primitive->IsEnableShadowState();
	}
	return false;
}
 
bool UCompassMarker::IsEnableDistanceSwitching()
{ 
	if (const FMarkerPrimitive* const Primitive = FindMarkerPrimitive())
	{
		return Primitive->IsEnableDistanceSwitching();
	}
	return false;
}
 
bool UCompassMarker::SetEnableShadowState(const bool bEnable)
{
	if (FMarkerPrimitive* Primitive = FindMarkerPrimitive())
	{
		if (Primitive->TryChangeShadowState(bEnable))
		{
			PostInitUpdate(*Primitive);
			return true;
		}
	}
	return false;
}
 
bool UCompassMarker::SetEnableShadow(const bool bEnable)
{
	if (FMarkerPrimitive* Primitive = FindMarkerPrimitive())
	{
		Primitive->SetEnableShadow(bEnable);
		PostInitUpdate(*Primitive);
		return true;
	}
	return false;
}
 
bool UCompassMarker::SetEnableDistanceSwitching(const bool bEnable)
{

	if (FMarkerPrimitive* Primitive = FindMarkerPrimitive())
	{
		Primitive->SetEnableDistanceSwitching(bEnable);
		PostInitUpdate(*Primitive);
		return true;
	}
	return false;
}

void UCompassMarker::PlayRemoveAnimation()
{
	if (IsPlayingAnimation())
	{
		return;
	}
	PlayAnimationForward(RemoveAnimation, RemoveAnimSpeed);
}

void UCompassMarker::FinishedRemoveAnimation()
{
	const UGameInstance* const GameInstance = UGameplayStatics::GetGameInstance(GetWorld());
	UCompassSubsystem* const CompassSubsystem = GameInstance->GetSubsystem<UCompassSubsystem>();
	CompassSubsystem->RemoveMarkerForce(MarkweID);
}

void UCompassMarker::OnAnimationFinished_Implementation(const UWidgetAnimation* Animation)
{
	if (Animation == RemoveAnimation)
	{
		FinishedRemoveAnimation();
	}
	Super::OnAnimationFinished_Implementation(Animation);
}
