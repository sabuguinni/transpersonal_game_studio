// BiomeTemperatureManager.h
// Procedural World Generator — Agent #05
// Manages biome temperature zones and integrates with SurvivalComponent.SetAmbientTemperature()
// Each biome trigger volume pushes ambient temperature to the player's SurvivalComponent on overlap.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "BiomeTemperatureManager.generated.h"

// Biome types matching the 5 zones in MinPlayableMap
UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Plains      UMETA(DisplayName = "Plains"),
    Volcanic    UMETA(DisplayName = "Volcanic"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Rocky       UMETA(DisplayName = "Rocky Highlands"),
    Unknown     UMETA(DisplayName = "Unknown")
};

// Per-biome configuration
USTRUCT(BlueprintType)
struct FWorld_BiomeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Unknown;

    // Ambient temperature in Celsius — fed to SurvivalComponent.SetAmbientTemperature()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome", meta = (ClampMin = "-50.0", ClampMax = "80.0"))
    float AmbientTemperatureCelsius = 22.0f;

    // Humidity 0-1 — affects thirst drain rate
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Humidity = 0.5f;

    // Danger level 0-1 — affects dinosaur spawn density
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DangerLevel = 0.3f;

    // Display name for UI
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName = TEXT("Unknown Biome");
};

// Delegate fired when player enters a new biome
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBiomeEntered, EWorld_BiomeType, NewBiome, FWorld_BiomeConfig, BiomeConfig);

/**
 * ABiomeTemperatureTrigger
 * Placed in the world as a trigger volume. When the player overlaps,
 * it calls SurvivalComponent->SetAmbientTemperature() with the biome's value.
 * Also broadcasts OnBiomeEntered for HUD/audio/VFX agents to react.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Biome Temperature Trigger"))
class TRANSPERSONALGAME_API ABiomeTemperatureTrigger : public AActor
{
    GENERATED_BODY()

public:
    ABiomeTemperatureTrigger();

protected:
    virtual void BeginPlay() override;

public:
    // Collision box — size set per biome in editor
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome", meta = (AllowPrivateAccess = "true"))
    UBoxComponent* TriggerBox;

    // Biome configuration for this zone
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FWorld_BiomeConfig BiomeConfig;

    // Fired when any pawn enters this biome zone
    UPROPERTY(BlueprintAssignable, Category = "Biome")
    FOnBiomeEntered OnBiomeEntered;

    // Called when player enters — pushes temperature to SurvivalComponent
    UFUNCTION(BlueprintCallable, Category = "Biome")
    void ApplyBiomeToActor(AActor* OverlappingActor);

    // Returns current biome config
    UFUNCTION(BlueprintPure, Category = "Biome")
    FWorld_BiomeConfig GetBiomeConfig() const { return BiomeConfig; }

    // Returns biome type
    UFUNCTION(BlueprintPure, Category = "Biome")
    EWorld_BiomeType GetBiomeType() const { return BiomeConfig.BiomeType; }

private:
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);
};
