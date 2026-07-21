#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NPCBehaviorComponent.h"
#include "../../SharedTypes.h"
#include "TribalNPC.generated.h"

UENUM(BlueprintType)
enum class ENPC_TribalRole : uint8
{
    Hunter,
    Gatherer,
    Crafter,
    Guard,
    Elder,
    Child,
    Shaman
};

USTRUCT(BlueprintType)
struct FNPC_TribalStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Stats")
    float Strength = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Stats")
    float Agility = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Stats")
    float Intelligence = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Stats")
    float Wisdom = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Stats")
    float Fear = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Stats")
    float Hunger = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Stats")
    float Fatigue = 0.0f;
};

USTRUCT(BlueprintType)
struct FNPC_Relationship
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    AActor* RelatedNPC = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    float Affection = 0.0f; // -100 to 100

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    float Trust = 0.0f; // -100 to 100

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    float Respect = 0.0f; // -100 to 100

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    FString RelationshipType = TEXT("Stranger");
};

UCLASS()
class TRANSPERSONALGAME_API ATribalNPC : public ACharacter
{
    GENERATED_BODY()

public:
    ATribalNPC();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // NPC Identity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    FString NPCName = TEXT("Tribal Member");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    ENPC_TribalRole TribalRole = ENPC_TribalRole::Gatherer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    int32 Age = 25;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    bool bIsMale = true;

    // Stats and Attributes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    FNPC_TribalStats TribalStats;

    // Behavior Component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UNPC_BehaviorComponent* BehaviorComponent;

    // Relationships
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    TArray<FNPC_Relationship> Relationships;

    // Tribal Functions
    UFUNCTION(BlueprintCallable, Category = "Tribal NPC")
    void SetTribalRole(ENPC_TribalRole NewRole);

    UFUNCTION(BlueprintPure, Category = "Tribal NPC")
    ENPC_TribalRole GetTribalRole() const { return TribalRole; }

    UFUNCTION(BlueprintCallable, Category = "Tribal NPC")
    void ModifyRelationship(AActor* OtherNPC, float AffectionChange, float TrustChange, float RespectChange);

    UFUNCTION(BlueprintPure, Category = "Tribal NPC")
    FNPC_Relationship GetRelationshipWith(AActor* OtherNPC);

    // Survival Functions
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ModifyHunger(float HungerChange);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ModifyFear(float FearChange);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ModifyFatigue(float FatigueChange);

    // Role-specific behaviors
    UFUNCTION(BlueprintCallable, Category = "Role Behavior")
    void ExecuteRoleBehavior();

    UFUNCTION(BlueprintCallable, Category = "Role Behavior")
    bool CanPerformRoleAction() const;

    // Communication
    UFUNCTION(BlueprintCallable, Category = "Communication")
    void SpeakToPlayer(const FString& Message);

    UFUNCTION(BlueprintCallable, Category = "Communication")
    void ReactToPlayerAction(const FString& ActionType);

protected:
    // Internal role behaviors
    void ExecuteHunterBehavior();
    void ExecuteGathererBehavior();
    void ExecuteCrafterBehavior();
    void ExecuteGuardBehavior();
    void ExecuteElderBehavior();
    void ExecuteChildBehavior();
    void ExecuteShamanBehavior();

    // Stat management
    void UpdateTribalStats(float DeltaTime);
    void HandleStatEffects();

    // Relationship management
    void UpdateRelationships(float DeltaTime);
    FNPC_Relationship* FindRelationship(AActor* OtherNPC);

    // Role-specific parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Role Parameters")
    float HuntingSkill = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Role Parameters")
    float GatheringSkill = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Role Parameters")
    float CraftingSkill = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Role Parameters")
    float CombatSkill = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Role Parameters")
    float WisdomLevel = 50.0f;

    // Timers and cooldowns
    UPROPERTY(BlueprintReadOnly, Category = "Internal")
    float RoleBehaviorTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Timing")
    float RoleBehaviorInterval = 30.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Internal")
    float StatUpdateTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Timing")
    float StatUpdateInterval = 5.0f;
};