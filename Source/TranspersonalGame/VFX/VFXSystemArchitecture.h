#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/ActorComponent.h"
#include "VFXSystemArchitecture.generated.h"

/**
 * VFX System Architecture for Transpersonal Game Studio
 * Jurassic Survival Game — Production Cycle PROD_JURASSIC_001
 * 
 * Core principle: VFX serves narrative and mechanics, never itself.
 * Every effect must enhance the feeling of being prey, not predator.
 */

UENUM(BlueprintType)
enum class EVFXCategory : uint8
{
    // Environmental effects that create atmosphere
    Atmospheric         UMETA(DisplayName = "Atmospheric"),
    
    // Effects that hint at dinosaur presence off-screen
    DinosaurPresence   UMETA(DisplayName = "Dinosaur Presence"),
    
    // Combat and impact effects
    Combat             UMETA(DisplayName = "Combat"),
    
    // Domestication and relationship building effects
    Domestication      UMETA(DisplayName = "Domestication"),
    
    // Survival mechanics (fire, tools, construction)
    Survival           UMETA(DisplayName = "Survival"),
    
    // Environmental destruction and physics
    Destruction        UMETA(DisplayName = "Destruction"),
    
    // Weather and time-of-day transitions
    Weather            UMETA(DisplayName = "Weather")
};

UENUM(BlueprintType)
enum class EVFXIntensity : uint8
{
    Subtle      UMETA(DisplayName = "Subtle"),      // Background atmosphere
    Moderate    UMETA(DisplayName = "Moderate"),    // Noticeable but not distracting
    Dramatic    UMETA(DisplayName = "Dramatic"),    // Key narrative moments
    Cinematic   UMETA(DisplayName = "Cinematic")    // Major story beats
};

UENUM(BlueprintType)
enum class EVFXPerformanceTier : uint8
{
    Low         UMETA(DisplayName = "Low Quality"),     // Mobile/Low-end PC
    Medium      UMETA(DisplayName = "Medium Quality"),  // Console baseline
    High        UMETA(DisplayName = "High Quality"),    // High-end PC
    Cinematic   UMETA(DisplayName = "Cinematic")        // Cutscenes only
};

USTRUCT(BlueprintType)
struct FVFXEffectDefinition
{
    GENERATED_BODY()

    // Effect identification
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Definition")
    FString EffectName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Definition")
    EVFXCategory Category;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Definition")
    EVFXIntensity Intensity;
    
    // Niagara system reference
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;
    
    // Performance scaling
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    TMap<EVFXPerformanceTier, float> QualityScaling;
    
    // Emotional intent
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intent")
    FString EmotionalIntent;
    
    // Usage context
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    TArray<FString> UsageContexts;
    
    FVFXEffectDefinition()
    {
        EffectName = TEXT("Unnamed Effect");
        Category = EVFXCategory::Atmospheric;
        Intensity = EVFXIntensity::Subtle;
        EmotionalIntent = TEXT("Enhance atmosphere");
        
        // Default performance scaling
        QualityScaling.Add(EVFXPerformanceTier::Low, 0.3f);
        QualityScaling.Add(EVFXPerformanceTier::Medium, 0.6f);
        QualityScaling.Add(EVFXPerformanceTier::High, 1.0f);
        QualityScaling.Add(EVFXPerformanceTier::Cinematic, 1.5f);
    }
};

/**
 * Master VFX Manager Component
 * Handles all VFX spawning, management, and performance optimization
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFXManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFXManagerComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Effect registry
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Registry")
    TMap<FString, FVFXEffectDefinition> EffectRegistry;
    
    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EVFXPerformanceTier CurrentPerformanceTier;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveEffects;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float EffectCullingDistance;
    
    // Active effects tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    TArray<UNiagaraComponent*> ActiveEffects;
    
    // Core VFX functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnEffect(const FString& EffectName, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator, AActor* AttachToActor = nullptr);
    
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopEffect(UNiagaraComponent* EffectComponent);
    
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllEffectsOfCategory(EVFXCategory Category);
    
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetPerformanceTier(EVFXPerformanceTier NewTier);
    
    // Atmospheric effects
    UFUNCTION(BlueprintCallable, Category = "Atmospheric VFX")
    void SpawnAtmosphericEffect(const FString& EffectName, const FVector& Location, float Duration = -1.0f);
    
    // Dinosaur presence hints
    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX")
    void TriggerDinosaurPresenceHint(const FVector& Location, float Intensity = 1.0f);
    
private:
    void UpdatePerformanceScaling();
    void CullDistantEffects();
    bool ShouldSpawnEffect(const FVFXEffectDefinition& EffectDef, const FVector& Location);
};