#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
#include "Narr_TribalSocialSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_TribalRank : uint8
{
    Outcast = 0,
    Newcomer = 1,
    Hunter = 2,
    Craftsman = 3,
    Warrior = 4,
    Scout = 5,
    Elder = 6,
    ChiefHunter = 7,
    TribalLeader = 8
};

UENUM(BlueprintType)
enum class ENarr_SocialRelation : uint8
{
    Hostile = 0,
    Suspicious = 1,
    Neutral = 2,
    Friendly = 3,
    Trusted = 4,
    Bonded = 5,
    Family = 6
};

UENUM(BlueprintType)
enum class ENarr_TribalActivity : uint8
{
    Hunting = 0,
    Gathering = 1,
    Crafting = 2,
    Storytelling = 3,
    Teaching = 4,
    Patrolling = 5,
    Resting = 6,
    Ritual = 7
};

USTRUCT(BlueprintType)
struct FNarr_TribalMember
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString Name;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    ENarr_TribalRank Rank;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int32 Age;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int32 ExperiencePoints;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float HuntingSkill;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float CraftingSkill;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float SurvivalKnowledge;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float Leadership;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TArray<FString> KnownStories;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    ENarr_TribalActivity CurrentActivity;

    FNarr_TribalMember()
    {
        Name = TEXT("Unknown");
        Rank = ENarr_TribalRank::Newcomer;
        Age = 25;
        ExperiencePoints = 0;
        HuntingSkill = 0.0f;
        CraftingSkill = 0.0f;
        SurvivalKnowledge = 0.0f;
        Leadership = 0.0f;
        CurrentActivity = ENarr_TribalActivity::Gathering;
    }
};

USTRUCT(BlueprintType)
struct FNarr_SocialBond
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString MemberA;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString MemberB;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    ENarr_SocialRelation RelationType;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float BondStrength;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString SharedExperience;

    FNarr_SocialBond()
    {
        MemberA = TEXT("");
        MemberB = TEXT("");
        RelationType = ENarr_SocialRelation::Neutral;
        BondStrength = 0.0f;
        SharedExperience = TEXT("");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_TribalSocialSystem : public AActor
{
    GENERATED_BODY()

public:
    ANarr_TribalSocialSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tribal System")
    TArray<FNarr_TribalMember> TribeMembers;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tribal System")
    TArray<FNarr_SocialBond> SocialBonds;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tribal System")
    FString TribeName;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tribal System")
    int32 TotalPopulation;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tribal System")
    float TribalMorale;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tribal System")
    float FoodSupply;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tribal System")
    float WaterSupply;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tribal System")
    float SafetyLevel;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Tribal System")
    void AddTribeMember(const FNarr_TribalMember& NewMember);

    UFUNCTION(BlueprintCallable, Category = "Tribal System")
    void RemoveTribeMember(const FString& MemberName);

    UFUNCTION(BlueprintCallable, Category = "Tribal System")
    FNarr_TribalMember GetTribeMember(const FString& MemberName);

    UFUNCTION(BlueprintCallable, Category = "Tribal System")
    void UpdateMemberRank(const FString& MemberName, ENarr_TribalRank NewRank);

    UFUNCTION(BlueprintCallable, Category = "Tribal System")
    void CreateSocialBond(const FString& MemberA, const FString& MemberB, ENarr_SocialRelation RelationType);

    UFUNCTION(BlueprintCallable, Category = "Tribal System")
    void UpdateSocialBond(const FString& MemberA, const FString& MemberB, float BondChange);

    UFUNCTION(BlueprintCallable, Category = "Tribal System")
    TArray<FNarr_TribalMember> GetMembersByRank(ENarr_TribalRank Rank);

    UFUNCTION(BlueprintCallable, Category = "Tribal System")
    TArray<FNarr_TribalMember> GetMembersByActivity(ENarr_TribalActivity Activity);

    UFUNCTION(BlueprintCallable, Category = "Tribal System")
    void UpdateTribalMorale(float MoraleChange);

    UFUNCTION(BlueprintCallable, Category = "Tribal System")
    void UpdateSupplies(float FoodChange, float WaterChange);

    UFUNCTION(BlueprintCallable, Category = "Tribal System")
    void UpdateSafetyLevel(float SafetyChange);

    UFUNCTION(BlueprintCallable, Category = "Tribal System")
    FString GetTribalStatus();

    UFUNCTION(BlueprintCallable, Category = "Tribal System")
    void InitializeDefaultTribe();

    UFUNCTION(BlueprintCallable, Category = "Tribal System")
    void ProcessDailyTribalActivities();

    UFUNCTION(BlueprintCallable, Category = "Tribal System")
    TArray<FString> GetAvailableStories();

    UFUNCTION(BlueprintCallable, Category = "Tribal System")
    void ShareStoryWithTribe(const FString& StoryTeller, const FString& Story);
};