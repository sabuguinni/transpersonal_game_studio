#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "GameFramework/Actor.h"
#include "EnvArt_EnvironmentalStorytellingSystem.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_StorytellingZoneType : uint8
{
    AncientBattleground     UMETA(DisplayName = "Ancient Battleground"),
    PredatorTerritory       UMETA(DisplayName = "Predator Territory"),
    MigrationPath           UMETA(DisplayName = "Migration Path"),
    FeedingGround           UMETA(DisplayName = "Feeding Ground"),
    NestingSite             UMETA(DisplayName = "Nesting Site"),
    WaterSource             UMETA(DisplayName = "Water Source"),
    ShelterCave             UMETA(DisplayName = "Shelter Cave"),
    DeathSite               UMETA(DisplayName = "Death Site")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_StorytellingZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    EEnvArt_StorytellingZoneType ZoneType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    TArray<AActor*> StorytellingProps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    FString NarrativeDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    float AtmosphericIntensity;

    FEnvArt_StorytellingZone()
    {
        ZoneType = EEnvArt_StorytellingZoneType::AncientBattleground;
        CenterLocation = FVector::ZeroVector;
        Radius = 1000.0f;
        NarrativeDescription = TEXT("An area with environmental story elements");
        AtmosphericIntensity = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_AtmosphericNarrative
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString StoryElement;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FVector> PropLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FLinearColor AtmosphericColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float LightingIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bRequiresParticleEffects;

    FEnvArt_AtmosphericNarrative()
    {
        StoryElement = TEXT("Environmental story element");
        AtmosphericColor = FLinearColor::White;
        LightingIntensity = 1.0f;
        bRequiresParticleEffects = false;
    }
};

/**
 * Environmental Storytelling System
 * Creates narrative zones through environmental props and atmospheric effects
 * Tells stories without dialogue through visual environmental cues
 */
UCLASS(ClassGroup=(Environment), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEnvArt_EnvironmentalStorytellingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnvArt_EnvironmentalStorytellingSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Storytelling Zone Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Storytelling")
    TArray<FEnvArt_StorytellingZone> StorytellingZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Storytelling")
    TArray<FEnvArt_AtmosphericNarrative> AtmosphericNarratives;

    // Zone Creation Functions
    UFUNCTION(BlueprintCallable, Category = "Environmental Storytelling")
    void CreateAncientBattlegroundZone(const FVector& Location, float Radius = 2000.0f);

    UFUNCTION(BlueprintCallable, Category = "Environmental Storytelling")
    void CreatePredatorTerritoryZone(const FVector& Location, float Radius = 1500.0f);

    UFUNCTION(BlueprintCallable, Category = "Environmental Storytelling")
    void CreateMigrationPathZone(const FVector& StartLocation, const FVector& EndLocation);

    UFUNCTION(BlueprintCallable, Category = "Environmental Storytelling")
    void CreateFeedingGroundZone(const FVector& Location, float Radius = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Environmental Storytelling")
    void CreateNestingSiteZone(const FVector& Location, float Radius = 800.0f);

    // Atmospheric Storytelling
    UFUNCTION(BlueprintCallable, Category = "Environmental Storytelling")
    void ApplyAtmosphericNarrative(const FEnvArt_AtmosphericNarrative& Narrative);

    UFUNCTION(BlueprintCallable, Category = "Environmental Storytelling")
    void EnhanceLightingForStorytelling(const FVector& FocusLocation, const FLinearColor& AtmosphericColor);

    // Prop Spawning for Storytelling
    UFUNCTION(BlueprintCallable, Category = "Environmental Storytelling")
    void SpawnStorytellingProps(EEnvArt_StorytellingZoneType ZoneType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Environmental Storytelling")
    void SpawnBrokenTreeStumps(const FVector& CenterLocation, int32 Count = 3);

    UFUNCTION(BlueprintCallable, Category = "Environmental Storytelling")
    void SpawnTerritorialMarkers(const FVector& CenterLocation, int32 Count = 4);

    UFUNCTION(BlueprintCallable, Category = "Environmental Storytelling")
    void SpawnFeedingGroundIndicators(const FVector& CenterLocation, int32 Count = 5);

    UFUNCTION(BlueprintCallable, Category = "Environmental Storytelling")
    void SpawnNestRemains(const FVector& Location);

    // Atmospheric Effects
    UFUNCTION(BlueprintCallable, Category = "Environmental Storytelling")
    void CreateAtmosphericDustEffect(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Environmental Storytelling")
    void CreatePollenParticleEffect(const FVector& Location);

    // Editor Functions
    UFUNCTION(CallInEditor, Category = "Environmental Storytelling")
    void GenerateAllStorytellingZones();

    UFUNCTION(CallInEditor, Category = "Environmental Storytelling")
    void ClearAllStorytellingElements();

    UFUNCTION(CallInEditor, Category = "Environmental Storytelling")
    void ValidateStorytellingZones();

protected:
    // Internal helper functions
    void SpawnStorytellingProp(const FVector& Location, const FString& PropLabel);
    void SetupZoneAtmosphere(const FEnvArt_StorytellingZone& Zone);
    void UpdateAtmosphericEffects(float DeltaTime);

    // Zone validation
    bool IsValidStorytellingLocation(const FVector& Location) const;
    float CalculateZoneInfluence(const FVector& PlayerLocation, const FEnvArt_StorytellingZone& Zone) const;

private:
    // Runtime tracking
    UPROPERTY()
    TArray<AActor*> SpawnedStorytellingActors;

    UPROPERTY()
    float CurrentAtmosphericIntensity;

    UPROPERTY()
    bool bStorytellingSystemActive;
};