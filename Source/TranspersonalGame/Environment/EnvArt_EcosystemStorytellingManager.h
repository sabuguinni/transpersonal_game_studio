#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/TriggerVolume.h"
#include "SharedTypes.h"
#include "EnvArt_EcosystemStorytellingManager.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_StorytellingElementType : uint8
{
    AncientBones        UMETA(DisplayName = "Ancient Bones"),
    AbandonedNest       UMETA(DisplayName = "Abandoned Nest"),
    ClawMarks           UMETA(DisplayName = "Claw Marks"),
    FeedingGround       UMETA(DisplayName = "Feeding Ground"),
    DisturbedGround     UMETA(DisplayName = "Disturbed Ground"),
    FreshTracks         UMETA(DisplayName = "Fresh Tracks"),
    BrokenBranches      UMETA(DisplayName = "Broken Branches")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_StorytellingElement
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    EEnvArt_StorytellingElementType ElementType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    FVector WorldLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    FRotator WorldRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    float InteractionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    bool bIsInteractive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    FString StoryDescription;

    FEnvArt_StorytellingElement()
    {
        ElementType = EEnvArt_StorytellingElementType::AncientBones;
        WorldLocation = FVector::ZeroVector;
        WorldRotation = FRotator::ZeroRotator;
        InteractionRadius = 200.0f;
        bIsInteractive = true;
        StoryDescription = TEXT("An ancient remnant tells a story of the past...");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_AtmosphericParticleConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FVector SpawnLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FString ParticleType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float IntensityMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float LifetimeMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    bool bAffectedByWind;

    FEnvArt_AtmosphericParticleConfig()
    {
        SpawnLocation = FVector::ZeroVector;
        ParticleType = TEXT("spores_floating");
        IntensityMultiplier = 1.0f;
        LifetimeMultiplier = 1.0f;
        bAffectedByWind = true;
    }
};

/**
 * Manages environmental storytelling elements and atmospheric effects
 * Creates immersive ecosystem with interactive props that tell stories
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_EcosystemStorytellingManager : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_EcosystemStorytellingManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // === STORYTELLING ELEMENTS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    TArray<FEnvArt_StorytellingElement> StorytellingElements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    float StorytellingUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    bool bEnableInteractiveElements;

    // === ATMOSPHERIC PARTICLES ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    TArray<FEnvArt_AtmosphericParticleConfig> ParticleConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AtmosphericIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    bool bDynamicAtmosphere;

    // === ECOSYSTEM SOUNDS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TArray<FVector> EcosystemSoundLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AmbientSoundRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float SoundVolumeMultiplier;

public:
    // === STORYTELLING FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void SpawnStorytellingElement(const FEnvArt_StorytellingElement& Element);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void UpdateStorytellingElements();

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void OnPlayerInteractWithElement(EEnvArt_StorytellingElementType ElementType, const FVector& Location);

    // === ATMOSPHERIC FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SpawnAtmosphericParticles(const FEnvArt_AtmosphericParticleConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateAtmosphericEffects(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetAtmosphericIntensity(float NewIntensity);

    // === ECOSYSTEM AUDIO FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SpawnEcosystemSounds();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateAmbientSounds(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetSoundVolumeMultiplier(float NewMultiplier);

    // === PERFORMANCE OPTIMIZATION ===
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeEcosystemPerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetActiveElementCount() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceLevel(int32 Level);

private:
    // Internal tracking
    float LastUpdateTime;
    int32 ActiveElementCount;
    bool bPerformanceOptimized;

    // Helper functions
    void InitializeStorytellingElements();
    void InitializeAtmosphericSystems();
    void InitializeEcosystemAudio();
    bool IsElementInRange(const FEnvArt_StorytellingElement& Element, const FVector& PlayerLocation) const;
    void UpdateElementVisibility(float DeltaTime);
};