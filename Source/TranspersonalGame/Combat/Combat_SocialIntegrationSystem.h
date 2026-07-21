#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "../AI/NPC_SocialBehaviorSystem.h"
#include "Combat_SocialIntegrationSystem.generated.h"

UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "No Threat"),
    Low         UMETA(DisplayName = "Low Threat"),
    Medium      UMETA(DisplayName = "Medium Threat"),
    High        UMETA(DisplayName = "High Threat"),
    Critical    UMETA(DisplayName = "Critical Threat")
};

UENUM(BlueprintType)
enum class ECombat_AllianceType : uint8
{
    Neutral     UMETA(DisplayName = "Neutral"),
    Friendly    UMETA(DisplayName = "Friendly"),
    Allied      UMETA(DisplayName = "Allied"),
    Hostile     UMETA(DisplayName = "Hostile"),
    Enemy       UMETA(DisplayName = "Enemy")
};

UENUM(BlueprintType)
enum class ECombat_CombatRole : uint8
{
    Defender    UMETA(DisplayName = "Defender"),
    Attacker    UMETA(DisplayName = "Attacker"),
    Support     UMETA(DisplayName = "Support"),
    Scout       UMETA(DisplayName = "Scout"),
    Leader      UMETA(DisplayName = "Leader")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_SocialCombatData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Combat")
    ECombat_ThreatLevel ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Combat")
    ECombat_AllianceType AllianceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Combat")
    ECombat_CombatRole CombatRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Combat")
    float TrustLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Combat")
    float FearLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Combat")
    float AggressionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Combat")
    int32 CombatExperience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Combat")
    bool bIsInCombat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Combat")
    bool bCanCallForHelp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Combat")
    float LastCombatTime;

    FCombat_SocialCombatData()
    {
        ThreatLevel = ECombat_ThreatLevel::None;
        AllianceType = ECombat_AllianceType::Neutral;
        CombatRole = ECombat_CombatRole::Defender;
        TrustLevel = 50.0f;
        FearLevel = 0.0f;
        AggressionLevel = 30.0f;
        CombatExperience = 0;
        bIsInCombat = false;
        bCanCallForHelp = true;
        LastCombatTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalFormation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Formation")
    FString FormationName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Formation")
    TArray<FVector> Positions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Formation")
    TArray<ECombat_CombatRole> Roles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Formation")
    float FormationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Formation")
    int32 MinParticipants;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Formation")
    int32 MaxParticipants;

    FCombat_TacticalFormation()
    {
        FormationName = "Default";
        FormationRadius = 500.0f;
        MinParticipants = 2;
        MaxParticipants = 8;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_SocialIntegrationSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_SocialIntegrationSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Social Combat Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Combat")
    FCombat_SocialCombatData SocialCombatData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Combat")
    TArray<AActor*> KnownAllies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Combat")
    TArray<AActor*> KnownEnemies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Combat")
    TArray<AActor*> NeutralActors;

    // Tactical Formations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Combat")
    TArray<FCombat_TacticalFormation> AvailableFormations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Combat")
    FCombat_TacticalFormation CurrentFormation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Combat")
    TArray<AActor*> FormationMembers;

    // Combat Decision Making
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float DetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float CombatRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float RetreatThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float CallForHelpRange;

    // Social Integration Functions
    UFUNCTION(BlueprintCallable, Category = "Social Combat")
    void UpdateSocialRelationship(AActor* OtherActor, float TrustDelta, float FearDelta);

    UFUNCTION(BlueprintCallable, Category = "Social Combat")
    ECombat_AllianceType DetermineAllianceType(AActor* OtherActor);

    UFUNCTION(BlueprintCallable, Category = "Social Combat")
    bool ShouldEngageInCombat(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Social Combat")
    bool ShouldCallForHelp();

    UFUNCTION(BlueprintCallable, Category = "Social Combat")
    void CallForHelp(ECombat_ThreatLevel ThreatLevel);

    // Tactical Formation Functions
    UFUNCTION(BlueprintCallable, Category = "Tactical Combat")
    bool FormTacticalFormation(const FString& FormationName);

    UFUNCTION(BlueprintCallable, Category = "Tactical Combat")
    void UpdateFormationPositions();

    UFUNCTION(BlueprintCallable, Category = "Tactical Combat")
    FVector GetFormationPosition(int32 MemberIndex);

    UFUNCTION(BlueprintCallable, Category = "Tactical Combat")
    void AddFormationMember(AActor* NewMember);

    UFUNCTION(BlueprintCallable, Category = "Tactical Combat")
    void RemoveFormationMember(AActor* Member);

    // Combat Intelligence Functions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    TArray<AActor*> FindNearbyAllies(float SearchRadius);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    TArray<AActor*> FindNearbyEnemies(float SearchRadius);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    AActor* SelectBestTarget();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector CalculateOptimalPosition(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldRetreat();

    // Memory and Learning
    UFUNCTION(BlueprintCallable, Category = "Combat Memory")
    void RecordCombatExperience(AActor* Opponent, bool bVictorious);

    UFUNCTION(BlueprintCallable, Category = "Combat Memory")
    float GetCombatEffectiveness(AActor* Opponent);

    UFUNCTION(BlueprintCallable, Category = "Combat Memory")
    void UpdateThreatAssessment(AActor* Actor, ECombat_ThreatLevel NewThreatLevel);

private:
    // Internal Functions
    void InitializeFormations();
    void UpdateSocialCombatState(float DeltaTime);
    void ProcessNearbyActors();
    bool IsActorInRange(AActor* Actor, float Range);
    float CalculateThreatScore(AActor* Actor);
    void BroadcastCombatAlert(ECombat_ThreatLevel ThreatLevel);

    // Reference to social behavior system
    UPROPERTY()
    class UNPC_SocialBehaviorSystem* SocialBehaviorSystem;

    // Combat state tracking
    float LastThreatCheck;
    float LastFormationUpdate;
    float CombatStartTime;
    
    TMap<AActor*, FCombat_SocialCombatData> ActorCombatData;
    TMap<AActor*, float> LastInteractionTimes;
};