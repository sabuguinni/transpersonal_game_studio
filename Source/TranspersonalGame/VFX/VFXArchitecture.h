#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/ActorComponent.h"
#include "VFXArchitecture.generated.h"

/**
 * VFX Architecture for Transpersonal Jurassic Survival Game
 * 
 * Core principle: VFX serves narrative tension and mechanical clarity.
 * Every effect must either:
 * 1. Amplify the "prey vs predator" dynamic
 * 2. Provide clear mechanical feedback
 * 3. Enhance environmental storytelling
 */

UENUM(BlueprintType)
enum class EVFXCategory : uint8
{
    // Environmental storytelling
    EnvironmentalAmbient     UMETA(DisplayName = "Environmental Ambient"),
    WeatherEffects          UMETA(DisplayName = "Weather Effects"),
    VegetationInteraction   UMETA(DisplayName = "Vegetation Interaction"),
    
    // Creature behavior feedback
    DinosaurBreathing       UMETA(DisplayName = "Dinosaur Breathing"),
    DinosaurMovement        UMETA(DisplayName = "Dinosaur Movement"),
    DinosaurEmotional       UMETA(DisplayName = "Dinosaur Emotional States"),
    
    // Player interaction
    PlayerMovement          UMETA(DisplayName = "Player Movement"),
    CraftingEffects         UMETA(DisplayName = "Crafting Effects"),
    ToolUsage              UMETA(DisplayName = "Tool Usage"),
    
    // Tension amplifiers
    DangerIndicators        UMETA(DisplayName = "Danger Indicators"),
    StealthFeedback         UMETA(DisplayName = "Stealth Feedback"),
    ThreatProximity         UMETA(DisplayName = "Threat Proximity"),
    
    // Combat and survival
    ImpactEffects           UMETA(DisplayName = "Impact Effects"),
    BloodEffects            UMETA(DisplayName = "Blood Effects"),
    DestructionEffects      UMETA(DisplayName = "Destruction Effects"),
    
    // Special moments
    GemEffects              UMETA(DisplayName = "Gem Effects"),
    DomesticationEffects    UMETA(DisplayName = "Domestication Effects"),
    TimeTransition          UMETA(DisplayName = "Time Transition")
};

UENUM(BlueprintType)
enum class EVFXIntensity : uint8
{
    Subtle      UMETA(DisplayName = "Subtle"),      // Barely noticeable, atmospheric
    Moderate    UMETA(DisplayName = "Moderate"),    // Clear but not overwhelming
    Prominent   UMETA(DisplayName = "Prominent"),   // Clearly visible, important moment
    Dramatic    UMETA(DisplayName = "Dramatic")     // High impact, critical moments
};

UENUM(BlueprintType)
enum class EVFXLODLevel : uint8
{
    High        UMETA(DisplayName = "High Quality"),     // Close range, high detail
    Medium      UMETA(DisplayName = "Medium Quality"),   // Mid range, balanced
    Low         UMETA(DisplayName = "Low Quality")       // Far range, performance optimized
};

USTRUCT(BlueprintType)
struct FVFXDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Definition")
    FString EffectName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Definition")
    EVFXCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Definition")
    EVFXIntensity Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Definition")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem_High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Definition")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem_Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Definition")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem_Low;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Definition")
    float MaxDrawDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Definition")
    bool bRequiresPlayerProximity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Definition")
    float PlayerProximityRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Definition")
    FString EmotionalIntent;

    FVFXDefinition()
    {
        EffectName = TEXT("");
        Category = EVFXCategory::EnvironmentalAmbient;
        Intensity = EVFXIntensity::Subtle;
        MaxDrawDistance = 5000.0f;
        bRequiresPlayerProximity = false;
        PlayerProximityRadius = 1000.0f;
        EmotionalIntent = TEXT("");
    }
};

/**
 * VFX Manager Component
 * Handles LOD switching, performance optimization, and effect triggering
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFXManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFXManagerComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core VFX management
    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    void PlayVFXEffect(const FString& EffectName, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    void StopVFXEffect(const FString& EffectName);

    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    void SetVFXLODLevel(EVFXLODLevel NewLODLevel);

    // Performance management
    UFUNCTION(BlueprintCallable, Category = "VFX Performance")
    void UpdateVFXLODBasedOnDistance(APawn* PlayerPawn);

    UFUNCTION(BlueprintCallable, Category = "VFX Performance")
    void SetMaxActiveEffects(int32 MaxEffects);

    // Tension system integration
    UFUNCTION(BlueprintCallable, Category = "VFX Tension")
    void TriggerTensionEffect(EVFXCategory TensionType, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "VFX Tension")
    void SetEnvironmentalTension(float TensionLevel); // 0.0 = calm, 1.0 = maximum danger

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Database")
    TArray<FVFXDefinition> VFXDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    int32 MaxActiveEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    float LODUpdateInterval;

    UPROPERTY(BlueprintReadOnly, Category = "VFX State")
    EVFXLODLevel CurrentLODLevel;

    UPROPERTY(BlueprintReadOnly, Category = "VFX State")
    float CurrentEnvironmentalTension;

private:
    TMap<FString, UNiagaraComponent*> ActiveEffects;
    float LastLODUpdateTime;

    EVFXLODLevel CalculateLODLevel(float DistanceToPlayer);
    UNiagaraSystem* GetNiagaraSystemForLOD(const FVFXDefinition& VFXDef, EVFXLODLevel LODLevel);
    void CleanupInactiveEffects();
};