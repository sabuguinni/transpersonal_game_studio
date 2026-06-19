// BiomeTemperatureManager.cpp
// Procedural World Generator — Agent #05
// Implementation of ABiomeTemperatureTrigger
// Integrates with SurvivalComponent via SetAmbientTemperature()

#include "BiomeTemperatureManager.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

// Forward-declare SurvivalComponent to avoid hard module dependency
// When SurvivalComponent module is confirmed in Build.cs, replace with full #include
class USurvivalComponent;

ABiomeTemperatureTrigger::ABiomeTemperatureTrigger()
{
    PrimaryActorTick.bCanEverTick = false;

    // Root collision box — default 3000x3000x500 units (overridden per biome in editor)
    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    TriggerBox->SetBoxExtent(FVector(3000.0f, 3000.0f, 500.0f));
    TriggerBox->SetCollisionProfileName(TEXT("OverlapAll"));
    TriggerBox->SetGenerateOverlapEvents(true);
    RootComponent = TriggerBox;

    // Default biome config — Forest at 22°C
    BiomeConfig.BiomeType = EWorld_BiomeType::Forest;
    BiomeConfig.AmbientTemperatureCelsius = 22.0f;
    BiomeConfig.Humidity = 0.6f;
    BiomeConfig.DangerLevel = 0.3f;
    BiomeConfig.BiomeName = TEXT("Forest");
}

void ABiomeTemperatureTrigger::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap delegate
    TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ABiomeTemperatureTrigger::OnOverlapBegin);
}

void ABiomeTemperatureTrigger::OnOverlapBegin(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this)
    {
        return;
    }

    ApplyBiomeToActor(OtherActor);
}

void ABiomeTemperatureTrigger::ApplyBiomeToActor(AActor* OverlappingActor)
{
    if (!OverlappingActor)
    {
        return;
    }

    // Find SurvivalComponent on the overlapping actor (player character)
    // Using FindComponentByClass with forward-declared type via raw UActorComponent lookup
    // This avoids hard module dependency — SurvivalComponent registers itself by name
    UActorComponent* SurvivalComp = OverlappingActor->FindComponentByClass(
        UActorComponent::StaticClass()
    );

    // Walk all components looking for SurvivalComponent by class name
    // This is the safe cross-module integration pattern
    TArray<UActorComponent*> Components;
    OverlappingActor->GetComponents(Components);

    for (UActorComponent* Comp : Components)
    {
        if (Comp && Comp->GetClass()->GetName().Contains(TEXT("SurvivalComponent")))
        {
            // Call SetAmbientTemperature via reflection (safe cross-module call)
            FName FuncName = TEXT("SetAmbientTemperature");
            UFunction* SetTempFunc = Comp->FindFunction(FuncName);
            if (SetTempFunc)
            {
                struct FSetTempParams
                {
                    float NewTemperature;
                };
                FSetTempParams Params;
                Params.NewTemperature = BiomeConfig.AmbientTemperatureCelsius;
                Comp->ProcessEvent(SetTempFunc, &Params);

                UE_LOG(LogTemp, Log, TEXT("BiomeTrigger: Applied temperature %.1fC to %s in biome %s"),
                    BiomeConfig.AmbientTemperatureCelsius,
                    *OverlappingActor->GetName(),
                    *BiomeConfig.BiomeName);
            }
            break;
        }
    }

    // Broadcast biome entered event — HUD, audio, VFX agents bind to this
    OnBiomeEntered.Broadcast(BiomeConfig.BiomeType, BiomeConfig);
}
