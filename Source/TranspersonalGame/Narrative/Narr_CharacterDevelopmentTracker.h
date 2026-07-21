#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
#include "Narr_CharacterDevelopmentTracker.generated.h"

UENUM(BlueprintType)
enum class ENarr_CharacterTrait : uint8
{
    Survivor,       // Basic survival instincts
    Hunter,         // Skilled at hunting and tracking
    Crafter,        // Excellent tool and weapon creation
    Builder,        // Skilled at construction and shelter
    Leader,         // Natural leadership abilities
    Strategist,     // Tactical thinking and planning
    Diplomat,       // Social skills and negotiation
    Warrior         // Combat prowess and bravery
};

UENUM(BlueprintType)
enum class ENarr_CharacterGrowthArea : uint8
{
    PhysicalStrength,
    MentalResilience,
    SocialSkills,
    TechnicalSkills,
    SurvivalInstincts,
    LeadershipQualities,
    CombatProficiency,
    CraftingExpertise
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_CharacterTraitData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    ENarr_CharacterTrait Trait;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float SkillLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float ExperiencePoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TArray<FString> UnlockedAbilities;

    FNarr_CharacterTraitData()
    {
        Trait = ENarr_CharacterTrait::Survivor;
        SkillLevel = 0.0f;
        ExperiencePoints = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_CharacterDevelopmentProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Development")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Development")
    TArray<FNarr_CharacterTraitData> Traits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Development")
    TMap<ENarr_CharacterGrowthArea, float> GrowthAreas;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Development")
    TArray<FString> CharacterMoments;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Development")
    float OverallDevelopmentScore;

    FNarr_CharacterDevelopmentProfile()
    {
        CharacterName = TEXT("Unnamed Survivor");
        OverallDevelopmentScore = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_CharacterDevelopmentTracker : public AActor
{
    GENERATED_BODY()

public:
    ANarr_CharacterDevelopmentTracker();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Development")
    FNarr_CharacterDevelopmentProfile PlayerProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Development")
    TMap<ENarr_CharacterTrait, FString> TraitDescriptions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Development")
    TMap<ENarr_CharacterGrowthArea, FString> GrowthAreaDescriptions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Development")
    float DevelopmentUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Development")
    bool bTrackDevelopmentAutomatically;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Character Development")
    void AddTraitExperience(ENarr_CharacterTrait Trait, float Experience);

    UFUNCTION(BlueprintCallable, Category = "Character Development")
    void UpdateGrowthArea(ENarr_CharacterGrowthArea Area, float GrowthAmount);

    UFUNCTION(BlueprintCallable, Category = "Character Development")
    void RecordCharacterMoment(const FString& MomentDescription);

    UFUNCTION(BlueprintCallable, Category = "Character Development")
    float GetTraitLevel(ENarr_CharacterTrait Trait) const;

    UFUNCTION(BlueprintCallable, Category = "Character Development")
    float GetGrowthAreaProgress(ENarr_CharacterGrowthArea Area) const;

    UFUNCTION(BlueprintCallable, Category = "Character Development")
    TArray<FString> GetCharacterStory() const;

    UFUNCTION(BlueprintCallable, Category = "Character Development")
    FString GetDominantTrait() const;

    UFUNCTION(BlueprintCallable, Category = "Character Development")
    FString GenerateCharacterSummary() const;

    UFUNCTION(BlueprintCallable, Category = "Character Development")
    bool HasUnlockedAbility(ENarr_CharacterTrait Trait, const FString& AbilityName) const;

    UFUNCTION(BlueprintCallable, Category = "Character Development")
    void UnlockAbility(ENarr_CharacterTrait Trait, const FString& AbilityName);

    UFUNCTION(BlueprintCallable, Category = "Character Development")
    TArray<ENarr_CharacterTrait> GetTopTraits(int32 Count = 3) const;

private:
    void InitializeTraitDescriptions();
    void InitializeGrowthAreaDescriptions();
    void CalculateOverallDevelopment();
    void CheckForTraitLevelUps();
    void BroadcastDevelopmentUpdate();

    FTimerHandle DevelopmentUpdateTimer;
    float LastDevelopmentCheck;
};