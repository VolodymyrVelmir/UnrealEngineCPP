# UnrealEngineCPP

#include "CompassSubsystem.h"
#include "CompassSettings.h"
#include "CompassPanel.h"
#include "Blueprint/UserWidget.h"
#include "Components/PanelWidget.h"
#include "MarkerObjectInterface.h"

bool FMarkerPrimitive::TryChangeShadowState(const bool bEnable, bool Force )
{
	if (Force)
	{
		CollisionShadowCount = 0;
		SetEnableShadowState(bEnable);
		return true;
	}
	else if (bEnable)
	{
		++CollisionShadowCount;
		SetEnableShadowState(bEnable);
		return true;
	}
	else if (!bEnable)
	{
		--CollisionShadowCount;
		if (CollisionShadowCount <= 0)
		{
			SetEnableShadowState(false);
			return true;
		}
	}
	return false;
}
FVector FMarkerPrimitive::GetTargetLocation() 
{
	this;
	if (AActor* const Actor = Cast<AActor>(MarkerObject))
	{
		return Actor->GetActorLocation();
	}
	return FVector::ZeroVector;
}


void UCompassSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}
 
 
void UCompassSubsystem::LinkedCompassWidget(UCompassPanel* InCompass)
{
	CompassWidget = InCompass;
	for (auto& AutoPrimitive : MarkerPrimitives)
	{
		FMarkerPrimitive& Primitive = AutoPrimitive.Value;
		if (UCompassMarker* const MarkerWidget = Primitive.GetMarkerWidget())
		{
			UPanelWidget* ParentPanel = MarkerWidget->GetParent();
			if (!ParentPanel)
			{
				CompassWidget->AddMarcers(MarkerWidget);
			}
		}
	}
}
 
bool UCompassSubsystem::CreateMarker(UObject* MarkerTarget)
{
 
	auto MarkerInterface = Cast<IMarkerObjectInterface>(MarkerTarget);

	if (!MarkerInterface)
	{
		return false;
	}
	const FName MarkerID = MarkerInterface->Execute_GetMarkerID(MarkerTarget);
	if (MarkerPrimitives.Contains(MarkerID))
	{
		return false;
	}
	const EMarkerType MarkerType = MarkerInterface->Execute_GetMarkerType(MarkerTarget);
	
	const UCompassSettings* const Setting = GetDefault<UCompassSettings>();
	FCreateMarkerSettings CreateSettings;
	if (Setting && Setting->FindMarkerSettings(MarkerType, CreateSettings))
	{
		const bool bBeginKnown = CreateSettings.bStartKnown && MarkerInterface->Execute_GetBeginKnown(MarkerTarget);
		if (UCompassMarker* const MarkerWidget = CreateWidget<UCompassMarker>(GetWorld(), CreateSettings.MarkerClass))
		{
			FMarkerPrimitive MarkerPrimitive { MarkerID, MarkerTarget, MarkerType, CreateSettings.bEnableShadow,  bBeginKnown, MarkerWidget, CreateSettings };
			MarkerPrimitives.Add(MarkerID, MarkerPrimitive);

			FMarkerPrimitive* InPrimitive = MarkerPrimitives.Find(MarkerID);
			MarkerWidget->SetMarkerPrimitive(InPrimitive, MarkerID);
			if (CreateSettings.bEnableBattleMarker)
			{
				++BattleMarkerCount;
			}
			if (CompassWidget.Get())
			{
				CompassWidget->AddMarcers(MarkerWidget);
			}
		}
		return true;
	}
	return false;
}
 
bool UCompassSubsystem::RemoveMarkerUpdate(FMarkerPrimitive& Marker)
{
	const UCompassSettings* const Setting = GetDefault<UCompassSettings>();
	FCreateMarkerSettings CreateSettings;
	if (Setting && Setting->FindMarkerSettings(Marker.GetMarkerType(), CreateSettings))
	{
		bool Successful = !CreateSettings.bEnableBattleMarker;
		if (CreateSettings.bEnableBattleMarker)
		{
			--BattleMarkerCount;
			if (BattleMarkerCount <= 0)
			{
				AddCharacterVisiblePoint(GetCharacterVisiblePoint() * -1);
			}
			Successful = true;
		}
		if (CreateSettings.bEnableRemoveAnimation && Marker.GetMarkerWidget())
		{
			Marker.GetMarkerWidget()->PlayRemoveAnimation();
			Successful = false;
		}
		return Successful;
	}
	return false;
}

bool UCompassSubsystem::RemoveMarkerForce(const FName MarkerID)
{
	if (IsHaveMarker(MarkerID))
	{
		if (FMarkerPrimitive* const Primitive = MarkerPrimitives.Find(MarkerID))
		{
			if (UCompassMarker* const MarkerWidget = Primitive->GetMarkerWidget())
			{
				CompassWidget->RemoveMarcers(MarkerWidget);
				MarkerPrimitives.Remove(MarkerID);
				return true;
			}
		}
	}
	return false;
}

bool UCompassSubsystem::RemoveMarker(FName MarkerID)
{
	if (MarkerPrimitives.Contains(MarkerID))
	{
		if (CompassWidget.Get())
		{
			if (FMarkerPrimitive* const Primitive = MarkerPrimitives.Find(MarkerID))
			{
				if (UCompassMarker* const MarkerWidget = Primitive->GetMarkerWidget())
				{
					if (RemoveMarkerUpdate(*Primitive))
					{
						RemoveMarkerForce(MarkerID);
						return true;
					}
				}
			}
		}
	}
	return false;
}

bool UCompassSubsystem::SetMarkerWidget(FName MarkerID, bool bKnown)
{
	if (MarkerPrimitives.Contains(MarkerID))
	{
		if (CompassWidget.Get())
		{
			if (FMarkerPrimitive* const Primitive = MarkerPrimitives.Find(MarkerID))
			{
				Primitive->SetMarkerKnown(bKnown);
				if (Primitive->GetMarkerWidget())
				{
					Primitive->GetMarkerWidget()->UpdateMarker();
				}
				return true;
			}
		}
	}
	return false;
}

FMarkerPrimitive* UCompassSubsystem::FindMarkerPrimitive(const FName& MarkerID)
{
	return MarkerPrimitives.Find(MarkerID);
}

void UCompassSubsystem::AddCharacterVisiblePoint(const float Point)
{
	CharacterVisiblePoint += Point;
	bool bBorder = false;
	bool bBorderMax = false;
	if (CharacterVisiblePoint <= 0.f)
	{
		CharacterVisiblePoint = 0.f;
		bBorder = true;
	}
	else if (CharacterVisiblePoint >= 1.f)
	{
		CharacterVisiblePoint = 1.f;
		bBorder = true;
		bBorderMax = true;
	}
	OnUpdateCharacterEnemyVisibilyty.Broadcast(CharacterVisiblePoint, bBorder, bBorderMax);
}

bool UCompassSubsystem::IsHaveMarker(FName MarkerID)
{
	return MarkerPrimitives.Contains(MarkerID);
}

float UCompassSubsystem::GetCharacterVisiblePoint() const 
{
	return CharacterVisiblePoint;
}
