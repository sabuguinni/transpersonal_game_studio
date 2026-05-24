#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "NPCBehaviorTypes.h"
#include "NPCBehaviorComponent.h"
#include "NPC_TribalHuman.generated.h"

UENUM(BlueprintType)
enum class ENPC_TribalRole : uint8
{
    Hunter      UMETA(DisplayName = "Hunter"),
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Crafter     UMETA(DisplayName = "Crafter"),
    Elder       UMETA(DisplayName = "Elder"),
    Scout       UMETA(DisplayName = "Scout"),
    Warrior     UMETA(DisplayName = "Warrior")
};

UENUM(BlueprintType)
enum class ENPC_TribalMood : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Alert       UMETA(DisplayName = "Alert"),
    Fearful     UMETA(DisplayName = "Fearful"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Curious     UMETA(DisplayName = "Curious"),
    Tired       UMETA(DisplayName = "Tired")
};

USTRUCT(BlueprintType)
struct FNPC_TribalMemory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    FVector LastPlayerLocation;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    float LastPlayerSeen;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    bool bPlayerWasHostile;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    TArray<FVector> DangerZones;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    TArray<FVector> SafeZones;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    float LastMealTime;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    float LastRestTime;

    FNPC_TribalMemory()
    {
        LastPlayerLocation = FVector::ZeroVector;
        LastPlayerSeen = 0.0f;
        bPlayerWasHostile = false;
        LastMealTime = 0.0f;
        LastRestTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FNPC_TribalStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Stats")
    float Health;

    UPROPERTY(BlueprintReadWrite, Category = "Stats")
    float MaxHealth;

    UPROPERTY(BlueprintReadWrite, Category = "Stats")
    float Hunger;

    UPROPERTY(BlueprintReadWrite, Category = "Stats")
    float Thirst;

    UPROPERTY(BlueprintReadWrite, Category = "Stats")
    float Energy;

    UPROPERTY(BlueprintReadWrite, Category = "Stats")
    float Fear;

    UPROPERTY(BlueprintReadWrite, Category = "Stats")
    float Aggression;

    FNPC_TribalStats()
    {
        Health = 100.0f;
        MaxHealth = 100.0f;
        Hunger = 50.0f;
        Thirst = 50.0f;
        Energy = 100.0f;
        Fear = 0.0f;
        Aggression = 20.0f;
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API ANPC_TribalHuman : public ACharacter
{
    GENERATED_BODY()

public:
    ANPC_TribalHuman();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UNPCBehaviorComponent* BehaviorComponent;

    // Tribal Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal")
    ENPC_TribalRole TribalRole;

    UPROPERTY(BlueprintReadOnly, Category = "Tribal")
    ENPC_TribalMood CurrentMood;

    UPROPERTY(BlueprintReadOnly, Category = "Tribal")
    FNPC_TribalStats TribalStats;

    UPROPERTY(BlueprintReadOnly, Category = "Tribal")
    FNPC_TribalMemory Memory;

    // AI Behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float DetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float FleeRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float PatrolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    FVector HomeLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    bool bIsNocturnal;

    // Daily Routine
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    TArray<FNPC_DailyActivity> DailyRoutine;

    UPROPERTY(BlueprintReadOnly, Category = "Routine")
    int32 CurrentActivityIndex;

public:
    // Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Tribal")
    void SetTribalRole(ENPC_TribalRole NewRole);

    UFUNCTION(BlueprintCallable, Category = "Tribal")
    void UpdateMood(ENPC_TribalMood NewMood);

    UFUNCTION(BlueprintCallable, Category = "Tribal")
    void ReactToPlayer(AActor* Player, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Tribal")
    void UpdateStats(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Tribal")
    void PerformActivity(ENPC_ActivityType Activity);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void RememberPlayerLocation(FVector Location, bool bWasHostile);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void AddDangerZone(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void AddSafeZone(FVector Location);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Tribal")
    ENPC_TribalRole GetTribalRole() const { return TribalRole; }

    UFUNCTION(BlueprintPure, Category = "Tribal")
    ENPC_TribalMood GetCurrentMood() const { return CurrentMood; }

    UFUNCTION(BlueprintPure, Category = "Tribal")
    FNPC_TribalStats GetTribalStats() const { return TribalStats; }

    UFUNCTION(BlueprintPure, Category = "Tribal")
    bool IsPlayerInMemory() const;

    UFUNCTION(BlueprintPure, Category = "Tribal")
    FVector GetLastKnownPlayerLocation() const { return Memory.LastPlayerLocation; }

private:
    // Internal Functions
    void InitializeTribalHuman();
    void UpdateDailyRoutine(float DeltaTime);
    void ProcessFear(float DeltaTime);
    void ProcessNeeds(float DeltaTime);
    bool ShouldFleeFromPlayer(float Distance) const;
    bool ShouldApproachPlayer(float Distance) const;
    void ExecuteRoleSpecificBehavior();
};