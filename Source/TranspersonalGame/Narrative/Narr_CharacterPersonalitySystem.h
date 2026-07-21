#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Components/ActorComponent.h"
#include "Narr_CharacterPersonalitySystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_PersonalityTrait : uint8
{
    Cautious        UMETA(DisplayName = "Cautious"),
    Aggressive      UMETA(DisplayName = "Aggressive"),
    Analytical      UMETA(DisplayName = "Analytical"),
    Empathetic      UMETA(DisplayName = "Empathetic"),
    Pragmatic       UMETA(DisplayName = "Pragmatic"),
    Optimistic      UMETA(DisplayName = "Optimistic"),
    Pessimistic     UMETA(DisplayName = "Pessimistic"),
    Leader          UMETA(DisplayName = "Leader"),
    Follower        UMETA(DisplayName = "Follower"),
    Survivor        UMETA(DisplayName = "Survivor")
};

UENUM(BlueprintType)
enum class ENarr_EmotionalState : uint8
{
    Calm            UMETA(DisplayName = "Calm"),
    Anxious         UMETA(DisplayName = "Anxious"),
    Determined      UMETA(DisplayName = "Determined"),
    Fearful         UMETA(DisplayName = "Fearful"),
    Hopeful         UMETA(DisplayName = "Hopeful"),
    Desperate       UMETA(DisplayName = "Desperate"),
    Confident       UMETA(DisplayName = "Confident"),
    Exhausted       UMETA(DisplayName = "Exhausted"),
    Alert           UMETA(DisplayName = "Alert"),
    Focused         UMETA(DisplayName = "Focused")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_PersonalityProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    TArray<ENarr_PersonalityTrait> PrimaryTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    TArray<ENarr_PersonalityTrait> SecondaryTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    ENarr_EmotionalState CurrentEmotionalState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float StressLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float ConfidenceLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float SurvivalExperience;

    FNarr_PersonalityProfile()
    {
        CurrentEmotionalState = ENarr_EmotionalState::Calm;
        StressLevel = 0.0f;
        ConfidenceLevel = 0.5f;
        SurvivalExperience = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialoguePersonalityModifier
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_PersonalityTrait RequiredTrait;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_EmotionalState RequiredEmotionalState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueVariant;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float PriorityWeight;

    FNarr_DialoguePersonalityModifier()
    {
        RequiredTrait = ENarr_PersonalityTrait::Cautious;
        RequiredEmotionalState = ENarr_EmotionalState::Calm;
        PriorityWeight = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_CharacterPersonalityComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_CharacterPersonalityComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    FNarr_PersonalityProfile PersonalityProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    FString CharacterBackground;

    UFUNCTION(BlueprintCallable, Category = "Personality")
    void UpdateEmotionalState(ENarr_EmotionalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Personality")
    void ModifyStressLevel(float DeltaStress);

    UFUNCTION(BlueprintCallable, Category = "Personality")
    void ModifyConfidenceLevel(float DeltaConfidence);

    UFUNCTION(BlueprintCallable, Category = "Personality")
    bool HasPersonalityTrait(ENarr_PersonalityTrait Trait) const;

    UFUNCTION(BlueprintCallable, Category = "Personality")
    FString GetPersonalityBasedDialogue(const FString& BaseDialogue, const TArray<FNarr_DialoguePersonalityModifier>& Modifiers) const;

    UFUNCTION(BlueprintCallable, Category = "Personality")
    float GetReactionIntensity(ENarr_EmotionalState TriggerState) const;

protected:
    virtual void BeginPlay() override;

private:
    void ClampPersonalityValues();
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UNarr_CharacterPersonalitySystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality System")
    TMap<FString, FNarr_PersonalityProfile> CharacterPersonalities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality System")
    TArray<FNarr_DialoguePersonalityModifier> GlobalDialogueModifiers;

    UFUNCTION(BlueprintCallable, Category = "Personality System")
    void RegisterCharacterPersonality(const FString& CharacterID, const FNarr_PersonalityProfile& Profile);

    UFUNCTION(BlueprintCallable, Category = "Personality System")
    FNarr_PersonalityProfile GetCharacterPersonality(const FString& CharacterID) const;

    UFUNCTION(BlueprintCallable, Category = "Personality System")
    void UpdateCharacterEmotionalState(const FString& CharacterID, ENarr_EmotionalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Personality System")
    TArray<FString> GetCharactersWithTrait(ENarr_PersonalityTrait Trait) const;

    UFUNCTION(BlueprintCallable, Category = "Personality System")
    FString GeneratePersonalityBasedResponse(const FString& CharacterID, const FString& BaseResponse) const;

    UFUNCTION(BlueprintCallable, Category = "Personality System")
    void ProcessGlobalStressEvent(float StressImpact);

    UFUNCTION(BlueprintCallable, Category = "Personality System")
    void ProcessSurvivalExperienceGain(const FString& CharacterID, float ExperienceGain);

private:
    void InitializeDefaultPersonalities();
    float CalculatePersonalityCompatibility(const FNarr_PersonalityProfile& ProfileA, const FNarr_PersonalityProfile& ProfileB) const;
};