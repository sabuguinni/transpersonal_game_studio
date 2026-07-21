#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "NPCBehaviorComponent.h"
#include "NPC_TribalBehaviorComponent.h"
#include "NPC_EmotionalBehaviorSystem.h"
#include "NPC_SocialBehaviorSystem.h"
#include "../SharedTypes.h"
#include "NPC_PrimitiveHumanActor.generated.h"

UENUM(BlueprintType)
enum class ENPC_HumanAge : uint8
{
    Child = 0,
    Adolescent = 1,
    Adult = 2,
    Elder = 3
};

UENUM(BlueprintType)
enum class ENPC_HumanRole : uint8
{
    Gatherer = 0,
    Hunter = 1,
    Crafter = 2,
    Guard = 3,
    Shaman = 4,
    Leader = 5
};

UENUM(BlueprintType)
enum class ENPC_SurvivalNeed : uint8
{
    Food = 0,
    Water = 1,
    Shelter = 2,
    Safety = 3,
    Social = 4,
    Rest = 5
};

USTRUCT(BlueprintType)
struct FNPC_SurvivalStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Hunger = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Thirst = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Energy = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Fear = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Comfort = 50.0f;
};

USTRUCT(BlueprintType)
struct FNPC_DailySchedule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float WakeUpTime = 6.0f; // Hour of day (0-24)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float WorkStartTime = 7.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float MealTime = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float RestTime = 18.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float SleepTime = 22.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    FVector WorkLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    FVector RestLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    FVector ShelterLocation = FVector::ZeroVector;
};

/**
 * Primitive Human NPC Actor - represents early humans in the prehistoric world
 * Focuses on survival behaviors, tribal dynamics, and realistic human needs
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANPC_PrimitiveHumanActor : public ACharacter
{
    GENERATED_BODY()

public:
    ANPC_PrimitiveHumanActor();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core NPC Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    FString NPCName = "Primitive Human";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    ENPC_HumanAge Age = ENPC_HumanAge::Adult;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    ENPC_HumanRole Role = ENPC_HumanRole::Gatherer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    int32 TribeID = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    bool bIsMale = true;

    // Survival Stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    FNPC_SurvivalStats SurvivalStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    ENPC_SurvivalNeed CurrentPrimaryNeed = ENPC_SurvivalNeed::Food;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float SurvivalUpdateRate = 5.0f; // Seconds between survival updates

    // Daily Schedule
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    FNPC_DailySchedule DailySchedule;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    bool bFollowsSchedule = true;

    // Behavior Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UNPCBehaviorComponent* NPCBehaviorComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UNPC_TribalBehaviorComponent* TribalBehaviorComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UNPC_EmotionalBehaviorSystem* EmotionalBehaviorComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UNPC_SocialBehaviorSystem* SocialBehaviorComponent;

    // Detection Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* AwarenessRadius;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* InteractionRadius;

    // Visual Representation
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* BodyMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* ClothingMesh;

    // AI and Movement
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float WalkSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float RunSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float AwarenessRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float InteractionRange = 300.0f;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "State")
    FVector CurrentDestination = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsMovingToDestination = false;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float TimeOfDay = 8.0f; // Current time in hours (0-24)

private:
    // Internal timers
    float SurvivalUpdateTimer = 0.0f;
    float ScheduleCheckTimer = 0.0f;
    float BehaviorUpdateTimer = 0.0f;

public:
    // Core Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateSurvivalNeeds(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void CheckDailySchedule();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetDestination(const FVector& NewDestination);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void InteractWithTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void RespondToThreat(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void PerformRoleBasedActivity();

    // Survival Functions
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ConsumeFood(float FoodValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void DrinkWater(float WaterValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Rest(float RestValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void TakeDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    ENPC_SurvivalNeed GetMostUrgentNeed() const;

    // Social Functions
    UFUNCTION(BlueprintCallable, Category = "Social")
    void GreetNearbyNPCs();

    UFUNCTION(BlueprintCallable, Category = "Social")
    void ShareResource(ANPC_PrimitiveHumanActor* OtherNPC, float ResourceAmount);

    UFUNCTION(BlueprintCallable, Category = "Social")
    void CallForHelp();

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    float GetDistanceToPlayer() const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    bool IsPlayerNearby(float Range = 500.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    FVector GetRandomLocationInRadius(float Radius) const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void SetTimeOfDay(float NewTimeOfDay) { TimeOfDay = NewTimeOfDay; }

    // Event Responses
    UFUNCTION()
    void OnAwarenessOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteractionOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};