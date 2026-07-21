#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Narr_TribalFactionSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_FactionRole : uint8
{
    Elder,
    Scout,
    CraftMaster,
    Hunter,
    Gatherer,
    Warrior,
    Healer,
    Storyteller
};

UENUM(BlueprintType)
enum class ENarr_FactionRelationship : uint8
{
    Allied,
    Neutral,
    Suspicious,
    Hostile,
    Unknown
};

USTRUCT(BlueprintType)
struct FNarr_CharacterBackstory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    ENarr_FactionRole Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    int32 Age;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    int32 SurvivalExperience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString BackstoryText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TArray<FString> SpecialSkills;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TArray<FString> KnownLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString PersonalityTraits;

    FNarr_CharacterBackstory()
    {
        CharacterName = TEXT("Unknown");
        Role = ENarr_FactionRole::Hunter;
        Age = 25;
        SurvivalExperience = 5;
        BackstoryText = TEXT("");
        PersonalityTraits = TEXT("Cautious, Observant");
    }
};

USTRUCT(BlueprintType)
struct FNarr_FactionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
    FString FactionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
    TArray<FNarr_CharacterBackstory> Members;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
    FString Territory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
    TArray<FString> Resources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
    ENarr_FactionRelationship PlayerRelationship;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
    int32 TrustLevel;

    FNarr_FactionData()
    {
        FactionName = TEXT("Unknown Tribe");
        Territory = TEXT("Uncharted");
        PlayerRelationship = ENarr_FactionRelationship::Unknown;
        TrustLevel = 0;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_TribalFactionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_TribalFactionSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Faction Management
    UFUNCTION(BlueprintCallable, Category = "Faction System")
    void InitializeTribalFactions();

    UFUNCTION(BlueprintCallable, Category = "Faction System")
    FNarr_FactionData GetFactionData(const FString& FactionName);

    UFUNCTION(BlueprintCallable, Category = "Faction System")
    void UpdateFactionRelationship(const FString& FactionName, ENarr_FactionRelationship NewRelationship);

    UFUNCTION(BlueprintCallable, Category = "Faction System")
    void ModifyTrustLevel(const FString& FactionName, int32 TrustChange);

    // Character Backstory System
    UFUNCTION(BlueprintCallable, Category = "Character System")
    FNarr_CharacterBackstory GetCharacterBackstory(const FString& CharacterName);

    UFUNCTION(BlueprintCallable, Category = "Character System")
    void AddCharacterToFaction(const FString& FactionName, const FNarr_CharacterBackstory& Character);

    UFUNCTION(BlueprintCallable, Category = "Character System")
    TArray<FNarr_CharacterBackstory> GetFactionMembers(const FString& FactionName);

    // Relationship Dynamics
    UFUNCTION(BlueprintCallable, Category = "Relationship System")
    bool CanPlayerInteractWithCharacter(const FString& CharacterName);

    UFUNCTION(BlueprintCallable, Category = "Relationship System")
    FString GetRelationshipStatusText(const FString& FactionName);

    UFUNCTION(BlueprintCallable, Category = "Relationship System")
    void ProcessPlayerAction(const FString& ActionType, const FString& TargetFaction, int32 ImpactValue);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction Data")
    TMap<FString, FNarr_FactionData> TribalFactions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Data")
    TMap<FString, FNarr_CharacterBackstory> CharacterDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Settings")
    int32 MaxTrustLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Settings")
    int32 MinTrustLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Settings")
    float RelationshipUpdateInterval;

private:
    void CreateDefaultFactions();
    void CreateTribalCharacters();
    void SetupFactionRelationships();
    
    FTimerHandle RelationshipUpdateTimer;
    void UpdateFactionDynamics();
};