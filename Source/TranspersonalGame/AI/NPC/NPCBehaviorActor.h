#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "NPCBehaviorActor.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorType : uint8
{
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Hunter      UMETA(DisplayName = "Hunter"),
    Elder       UMETA(DisplayName = "Elder"),
    Scout       UMETA(DisplayName = "Scout"),
    Trader      UMETA(DisplayName = "Trader"),
    Guard       UMETA(DisplayName = "Guard")
};

UENUM(BlueprintType)
enum class ENPC_EmotionalState : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Alert       UMETA(DisplayName = "Alert"),
    Afraid      UMETA(DisplayName = "Afraid"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Curious     UMETA(DisplayName = "Curious"),
    Friendly    UMETA(DisplayName = "Friendly")
};

USTRUCT(BlueprintType)
struct FNPC_Memory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString EventType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Importance;

    FNPC_Memory()
    {
        Location = FVector::ZeroVector;
        EventType = TEXT("");
        Timestamp = 0.0f;
        Importance = 1.0f;
    }
};

UCLASS(Blueprintable)
class TRANSPERSONALGAME_API ANPCBehaviorActor : public AActor
{
    GENERATED_BODY()

public:
    ANPCBehaviorActor();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // CORE COMPONENTS
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* NPCMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* InteractionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* DetectionSphere;

    // NPC IDENTITY
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    ENPC_BehaviorType NPCType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    int32 Age;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    float Experience;

    // BEHAVIOR PARAMETERS
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float PatrolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float InteractionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float DetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float MovementSpeed;

    // EMOTIONAL STATE
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    ENPC_EmotionalState CurrentEmotion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float EmotionIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float EmotionDecayRate;

    // MEMORY SYSTEM
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FNPC_Memory> Memories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxMemories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MemoryDecayRate;

    // DAILY ROUTINE
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    FVector HomeLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    FVector WorkLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    float WorkStartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    float WorkEndTime;

    // SOCIAL RELATIONSHIPS
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    TMap<AActor*, float> Relationships;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float SocialRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    bool bCanTrade;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    bool bCanGiveQuests;

    // BEHAVIOR FUNCTIONS
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void StartPatrol();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void StopPatrol();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetEmotionalState(ENPC_EmotionalState NewEmotion, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void AddMemory(FVector Location, const FString& EventType, float Importance);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateRelationship(AActor* OtherActor, float RelationshipChange);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    bool CanInteractWith(AActor* OtherActor);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void PerformDailyRoutine();

    // DETECTION FUNCTIONS
    UFUNCTION(BlueprintCallable, Category = "Detection")
    TArray<AActor*> GetNearbyActors();

    UFUNCTION(BlueprintCallable, Category = "Detection")
    bool IsPlayerInRange();

    UFUNCTION(BlueprintCallable, Category = "Detection")
    bool HasLineOfSight(AActor* Target);

private:
    // INTERNAL STATE
    bool bIsPatrolling;
    FVector PatrolCenter;
    FVector CurrentTarget;
    float LastEmotionUpdate;
    float LastMemoryCleanup;
    float LastRoutineCheck;

    // INTERNAL FUNCTIONS
    void UpdateEmotionalState(float DeltaTime);
    void CleanupMemories();
    void CheckDailyRoutine();
    FVector GetRandomPatrolPoint();
};