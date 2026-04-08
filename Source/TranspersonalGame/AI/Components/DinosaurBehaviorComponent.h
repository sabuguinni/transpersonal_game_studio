#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "../Core/DinosaurBehaviorTypes.h"
#include "DinosaurBehaviorComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMoodChanged, EDinosaurMoodState, OldMood, EDinosaurMoodState, NewMood);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActivityChanged, EDinosaurActivity, OldActivity, EDinosaurActivity, NewActivity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMemoryAdded, FVector, Location, FGameplayTag, EventType, float, Intensity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDomesticationChanged, EDomesticationStage, OldStage, EDomesticationStage, NewStage);

UCLASS(ClassGroup=(AI), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurBehaviorComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === CORE PROPERTIES ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    FDataTableRowHandle SpeciesDataHandle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Individual")
    FString IndividualName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Individual")
    EDinosaurPersonality Personality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Individual")
    int32 Age; // In days

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Individual")
    bool bIsMale;

    // === CURRENT STATE ===
    
    UPROPERTY(BlueprintReadOnly, Category = "State")
    EDinosaurMoodState CurrentMood;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    EDinosaurActivity CurrentActivity;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    EDomesticationStage DomesticationStage;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FDinosaurNeeds CurrentNeeds;

    // === MEMORY SYSTEM ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxMemoryEntries;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    TArray<FDinosaurMemoryEntry> MemoryEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MemoryRetentionTime; // Hours

    // === RELATIONSHIPS ===
    
    UPROPERTY(BlueprintReadOnly, Category = "Relationships")
    TArray<FDinosaurRelationship> KnownActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationships")
    int32 MaxRelationships;

    // === ROUTINE SYSTEM ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    TArray<FDailyRoutineEntry> DailyRoutine;

    UPROPERTY(BlueprintReadOnly, Category = "Routine")
    int32 CurrentRoutineIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    float RoutineFlexibility; // 0-1, how much deviation is allowed

    // === DOMESTICATION SYSTEM ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domestication")
    float DomesticationProgress; // 0-100

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domestication")
    TWeakObjectPtr<AActor> BondedPlayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domestication")
    float TrustDecayRate; // Per hour when not interacting

    // === ENVIRONMENTAL AWARENESS ===
    
    UPROPERTY(BlueprintReadOnly, Category = "Awareness")
    TArray<TWeakObjectPtr<AActor>> PerceivedThreats;

    UPROPERTY(BlueprintReadOnly, Category = "Awareness")
    TArray<TWeakObjectPtr<AActor>> PerceivedFood;

    UPROPERTY(BlueprintReadOnly, Category = "Awareness")
    TArray<TWeakObjectPtr<AActor>> PerceivedWater;

    UPROPERTY(BlueprintReadOnly, Category = "Awareness")
    TArray<TWeakObjectPtr<AActor>> PerceivedShelter;

    // === EVENTS ===
    
    UPROPERTY(BlueprintAssignable)
    FOnMoodChanged OnMoodChanged;

    UPROPERTY(BlueprintAssignable)
    FOnActivityChanged OnActivityChanged;

    UPROPERTY(BlueprintAssignable)
    FOnMemoryAdded OnMemoryAdded;

    UPROPERTY(BlueprintAssignable)
    FOnDomesticationChanged OnDomesticationChanged;

    // === PUBLIC FUNCTIONS ===
    
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetMood(EDinosaurMoodState NewMood);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetActivity(EDinosaurActivity NewActivity);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void AddMemory(FVector Location, FGameplayTag EventType, float Intensity, AActor* AssociatedActor = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    TArray<FDinosaurMemoryEntry> GetMemoriesNear(FVector Location, float Radius) const;

    UFUNCTION(BlueprintCallable, Category = "Memory")
    TArray<FDinosaurMemoryEntry> GetMemoriesByType(FGameplayTag EventType) const;

    UFUNCTION(BlueprintCallable, Category = "Relationships")
    void UpdateRelationship(AActor* Actor, float AffinityChange, float FamiliarityChange);

    UFUNCTION(BlueprintCallable, Category = "Relationships")
    FDinosaurRelationship GetRelationshipWith(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "Domestication")
    void ProcessDomesticationInteraction(AActor* Player, float PositiveValue, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Routine")
    FDailyRoutineEntry GetCurrentRoutineEntry() const;

    UFUNCTION(BlueprintCallable, Category = "Routine")
    bool ShouldChangeActivity() const;

    UFUNCTION(BlueprintCallable, Category = "Needs")
    void UpdateNeeds(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Needs")
    EDinosaurActivity GetMostUrgentNeedActivity() const;

    UFUNCTION(BlueprintPure, Category = "Species")
    FDinosaurSpeciesData GetSpeciesData() const;

    UFUNCTION(BlueprintPure, Category = "Behavior")
    float GetPersonalityModifier(FGameplayTag TraitTag) const;

    UFUNCTION(BlueprintCallable, Category = "Awareness")
    void RegisterPerceivedActor(AActor* Actor, FGameplayTag PerceptionType);

    UFUNCTION(BlueprintCallable, Category = "Awareness")
    void UnregisterPerceivedActor(AActor* Actor, FGameplayTag PerceptionType);

    UFUNCTION(BlueprintPure, Category = "Awareness")
    bool IsActorPerceived(AActor* Actor, FGameplayTag PerceptionType) const;

protected:
    // === INTERNAL FUNCTIONS ===
    
    void InitializeFromSpeciesData();
    void UpdateMemoryDecay(float DeltaTime);
    void UpdateRelationshipDecay(float DeltaTime);
    void UpdateDomesticationDecay(float DeltaTime);
    void UpdateRoutineProgression();
    void EvaluateMoodChanges();
    
    FDailyRoutineEntry* FindCurrentRoutineEntry();
    void CleanupOldMemories();
    void CleanupOldRelationships();
    
    float CalculateNeedUrgency(float NeedValue) const;
    float GetTimeOfDay() const;

private:
    // === INTERNAL STATE ===
    
    float LastNeedsUpdateTime;
    float LastRoutineCheckTime;
    float LastMoodEvaluationTime;
    
    // Cached species data for performance
    UPROPERTY()
    FDinosaurSpeciesData* CachedSpeciesData;
};