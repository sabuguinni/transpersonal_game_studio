#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "NPC_TribalBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_TribalRole : uint8
{
    CampGuard       UMETA(DisplayName = "Camp Guard"),
    Gatherer        UMETA(DisplayName = "Gatherer"),
    Hunter          UMETA(DisplayName = "Hunter"),
    Crafter         UMETA(DisplayName = "Crafter"),
    Elder           UMETA(DisplayName = "Elder"),
    Child           UMETA(DisplayName = "Child")
};

UENUM(BlueprintType)
enum class ENPC_DailyActivity : uint8
{
    Sleeping        UMETA(DisplayName = "Sleeping"),
    Patrolling      UMETA(DisplayName = "Patrolling"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Socializing     UMETA(DisplayName = "Socializing"),
    Eating          UMETA(DisplayName = "Eating"),
    Resting         UMETA(DisplayName = "Resting"),
    Fleeing         UMETA(DisplayName = "Fleeing")
};

USTRUCT(BlueprintType)
struct FNPC_DailySchedule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float StartHour;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float EndHour;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    ENPC_DailyActivity Activity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    FVector TargetLocation;

    FNPC_DailySchedule()
    {
        StartHour = 8.0f;
        EndHour = 10.0f;
        Activity = ENPC_DailyActivity::Resting;
        TargetLocation = FVector::ZeroVector;
    }
};

USTRUCT(BlueprintType)
struct FNPC_TribalMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    AActor* RememberedActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector LastSeenLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float LastSeenTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    bool bIsThreat;

    FNPC_TribalMemory()
    {
        RememberedActor = nullptr;
        LastSeenLocation = FVector::ZeroVector;
        LastSeenTime = 0.0f;
        bIsThreat = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_TribalBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_TribalBehaviorComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Tribal role and identity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Identity")
    ENPC_TribalRole TribalRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Identity")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Identity")
    int32 Age;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Identity")
    float ExperienceLevel;

    // Daily schedule system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    TArray<FNPC_DailySchedule> DailySchedule;

    UPROPERTY(BlueprintReadOnly, Category = "Daily Routine")
    ENPC_DailyActivity CurrentActivity;

    UPROPERTY(BlueprintReadOnly, Category = "Daily Routine")
    float CurrentGameHour;

    // Memory and awareness system
    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    TArray<FNPC_TribalMemory> MemoryBank;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MemoryDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float SightRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float HearingRange;

    // Social relationships
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    TMap<AActor*, float> RelationshipValues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float TrustLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float FearLevel;

    // Behavior functions
    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    void UpdateCurrentActivity();

    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    void AddMemory(AActor* Actor, FVector Location, bool bThreat);

    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    void UpdateRelationship(AActor* OtherActor, float DeltaValue);

    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    bool ShouldFlee();

    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    FVector GetCurrentTargetLocation();

    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    void ReactToPlayer(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    void ReactToDinosaur(AActor* Dinosaur);

private:
    void CleanupOldMemories();
    void PerformRoleSpecificBehavior();
    void UpdateEmotionalState();
};