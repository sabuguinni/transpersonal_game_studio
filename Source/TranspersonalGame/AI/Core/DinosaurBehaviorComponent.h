#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DinosaurBehaviorTypes.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "DinosaurBehaviorComponent.generated.h"

class UBlackboardComponent;
class UBehaviorTreeComponent;
class AAIController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBehaviorChanged, EDinosaurBehaviorType, OldBehavior, EDinosaurBehaviorType, NewBehavior);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMoodChanged, EDinosaurMoodState, OldMood, EDinosaurMoodState, NewMood);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDomesticationChanged, EDomesticationLevel, OldLevel, EDomesticationLevel, NewLevel);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurBehaviorComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === CONFIGURATION ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species Config")
    UDataTable* SpeciesConfigTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species Config")
    FName SpeciesRowName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Individual Traits")
    FDinosaurPhysicalTraits PhysicalTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Individual Traits")
    FDinosaurPersonality Personality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Individual Traits")
    FString IndividualName;

    // === CURRENT STATE ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    EDinosaurBehaviorType CurrentBehavior;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    EDinosaurMoodState CurrentMood;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    EDomesticationLevel DomesticationLevel;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    float DomesticationProgress;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    AActor* CurrentTarget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    FVector CurrentDestination;

    // === NEEDS SYSTEM ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Needs", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Hunger = 50.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Needs", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Thirst = 50.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Needs", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Energy = 100.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Needs", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Social = 50.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Needs", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Safety = 100.0f;

    // === MEMORY SYSTEM ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Memory")
    TArray<FDinosaurMemory> Memories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory", meta = (ClampMin = "1", ClampMax = "100"))
    int32 MaxMemories = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory", meta = (ClampMin = "60.0", ClampMax = "3600.0"))
    float MemoryDecayTime = 300.0f;

    // === DAILY ROUTINE ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    TArray<FDailyRoutineEntry> DailyRoutine;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Routine")
    int32 CurrentRoutineIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float TimeScale = 1.0f;

    // === SOCIAL RELATIONSHIPS ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Social")
    TMap<AActor*, float> RelationshipValues;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Social")
    TArray<AActor*> PackMembers;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Social")
    AActor* PackLeader;

    // === EVENTS ===
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnBehaviorChanged OnBehaviorChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnMoodChanged OnMoodChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnDomesticationChanged OnDomesticationChanged;

    // === BEHAVIOR FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetBehavior(EDinosaurBehaviorType NewBehavior);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetMood(EDinosaurMoodState NewMood);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    bool CanPerformBehavior(EDinosaurBehaviorType BehaviorType) const;

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    EDinosaurBehaviorType GetHighestPriorityBehavior() const;

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void InterruptCurrentBehavior();

    // === NEEDS FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Needs")
    void ModifyNeed(const FString& NeedName, float Amount);

    UFUNCTION(BlueprintCallable, Category = "Needs")
    float GetNeedValue(const FString& NeedName) const;

    UFUNCTION(BlueprintCallable, Category = "Needs")
    FString GetMostUrgentNeed() const;

    // === MEMORY FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void AddMemory(FVector Location, AActor* Actor, FGameplayTag MemoryType, float EmotionalWeight);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    TArray<FDinosaurMemory> GetMemoriesOfType(FGameplayTag MemoryType) const;

    UFUNCTION(BlueprintCallable, Category = "Memory")
    FDinosaurMemory GetStrongestMemory() const;

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void ForgetMemoriesOlderThan(float TimeThreshold);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    bool HasMemoryOf(AActor* Actor) const;

    // === DOMESTICATION FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Domestication")
    void AddDomesticationProgress(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Domestication")
    bool CanBeDomesticated() const;

    UFUNCTION(BlueprintCallable, Category = "Domestication")
    float GetDomesticationThreshold(EDomesticationLevel Level) const;

    // === SOCIAL FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Social")
    void ModifyRelationship(AActor* OtherActor, float Amount);

    UFUNCTION(BlueprintCallable, Category = "Social")
    float GetRelationshipValue(AActor* OtherActor) const;

    UFUNCTION(BlueprintCallable, Category = "Social")
    void JoinPack(AActor* Leader);

    UFUNCTION(BlueprintCallable, Category = "Social")
    void LeavePack();

    // === ROUTINE FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Routine")
    void UpdateDailyRoutine();

    UFUNCTION(BlueprintCallable, Category = "Routine")
    FDailyRoutineEntry GetCurrentRoutineEntry() const;

    UFUNCTION(BlueprintCallable, Category = "Routine")
    float GetCurrentGameHour() const;

    // === UTILITY FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Utility")
    void InitializeFromSpeciesConfig();

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void GenerateRandomTraits();

    UFUNCTION(BlueprintCallable, Category = "Utility")
    FString GetBehaviorDescription() const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void UpdateBlackboardValues();

private:
    // Internal state management
    void UpdateNeeds(float DeltaTime);
    void UpdateMemories(float DeltaTime);
    void UpdateMood();
    void UpdateDomesticationLevel();
    void ProcessRoutineTransitions();
    
    // Cached references
    UPROPERTY()
    AAIController* CachedAIController;

    UPROPERTY()
    UBlackboardComponent* CachedBlackboard;

    UPROPERTY()
    UBehaviorTreeComponent* CachedBehaviorTree;

    // Species config cache
    UPROPERTY()
    FDinosaurSpeciesConfig* CachedSpeciesConfig;

    // Internal timers
    float LastNeedsUpdate;
    float LastMemoryUpdate;
    float LastMoodUpdate;
    float LastRoutineCheck;
};