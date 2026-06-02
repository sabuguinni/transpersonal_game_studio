#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "NPC_DinosaurPersonalitySystem.generated.h"

UENUM(BlueprintType)
enum class ENPC_PersonalityTrait : uint8
{
    Aggressive      UMETA(DisplayName = "Aggressive"),
    Docile          UMETA(DisplayName = "Docile"),
    Curious         UMETA(DisplayName = "Curious"),
    Territorial     UMETA(DisplayName = "Territorial"),
    Social          UMETA(DisplayName = "Social"),
    Solitary        UMETA(DisplayName = "Solitary"),
    Protective      UMETA(DisplayName = "Protective"),
    Skittish        UMETA(DisplayName = "Skittish"),
    Dominant        UMETA(DisplayName = "Dominant"),
    Submissive      UMETA(DisplayName = "Submissive")
};

USTRUCT(BlueprintType)
struct FNPC_PersonalityProfile
{
    GENERATED_BODY()

    // Core personality traits (0.0 to 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Aggressiveness = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Curiosity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Sociability = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Territoriality = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Fearfulness = 0.5f;

    // Behavioral modifiers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.1", ClampMax = "3.0"))
    float ActivityLevel = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.1", ClampMax = "3.0"))
    float AlertnessLevel = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.1", ClampMax = "3.0"))
    float LearningRate = 1.0f;

    FNPC_PersonalityProfile()
    {
        Aggressiveness = 0.5f;
        Curiosity = 0.5f;
        Sociability = 0.5f;
        Territoriality = 0.5f;
        Fearfulness = 0.5f;
        ActivityLevel = 1.0f;
        AlertnessLevel = 1.0f;
        LearningRate = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct FNPC_BehaviorModifier
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FString ModifierName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float IntensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float Duration = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsPermanent = false;

    FNPC_BehaviorModifier()
    {
        ModifierName = TEXT("Default");
        IntensityMultiplier = 1.0f;
        Duration = 60.0f;
        bIsPermanent = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_DinosaurPersonalitySystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_DinosaurPersonalitySystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Personality profile
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    FNPC_PersonalityProfile PersonalityProfile;

    // Dominant personality traits
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    TArray<ENPC_PersonalityTrait> DominantTraits;

    // Active behavior modifiers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    TArray<FNPC_BehaviorModifier> ActiveModifiers;

    // Species-specific personality templates
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    FString SpeciesType = TEXT("Generic");

    // Personality functions
    UFUNCTION(BlueprintCallable, Category = "Personality")
    void GenerateRandomPersonality();

    UFUNCTION(BlueprintCallable, Category = "Personality")
    void SetSpeciesPersonality(const FString& Species);

    UFUNCTION(BlueprintCallable, Category = "Personality")
    void AddPersonalityTrait(ENPC_PersonalityTrait Trait, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Personality")
    bool HasPersonalityTrait(ENPC_PersonalityTrait Trait) const;

    UFUNCTION(BlueprintCallable, Category = "Personality")
    float GetPersonalityTraitIntensity(ENPC_PersonalityTrait Trait) const;

    // Behavior modification
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void AddBehaviorModifier(const FString& ModifierName, float Intensity, float Duration, bool bPermanent = false);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void RemoveBehaviorModifier(const FString& ModifierName);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    float GetBehaviorModifierIntensity(const FString& ModifierName) const;

    // Personality-based decision making
    UFUNCTION(BlueprintCallable, Category = "Decision Making")
    bool ShouldApproachTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Decision Making")
    bool ShouldFleeFromThreat(AActor* Threat) const;

    UFUNCTION(BlueprintCallable, Category = "Decision Making")
    bool ShouldDefendTerritory(AActor* Intruder) const;

    UFUNCTION(BlueprintCallable, Category = "Decision Making")
    bool ShouldJoinGroup(AActor* GroupLeader) const;

    // Personality evolution over time
    UFUNCTION(BlueprintCallable, Category = "Evolution")
    void EvolvePersonality(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Evolution")
    void LearnFromExperience(const FString& ExperienceType, float Impact);

    // Debugging and information
    UFUNCTION(BlueprintCallable, Category = "Debug")
    FString GetPersonalityDescription() const;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    TArray<FString> GetActiveModifiersNames() const;

private:
    // Internal personality evolution
    float PersonalityEvolutionTimer = 0.0f;
    TMap<FString, float> ExperienceMemory;

    // Helper functions
    void UpdateBehaviorModifiers(float DeltaTime);
    void ApplySpeciesDefaults(const FString& Species);
    float CalculatePersonalityInfluence(ENPC_PersonalityTrait Trait) const;
    void NormalizePersonalityProfile();
};