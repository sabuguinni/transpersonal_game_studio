#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DinosaurBehaviorTypes.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "DinosaurBehaviorComponent.generated.h"

class UBlackboardComponent;
class UBehaviorTreeComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDinosaurMoodChanged, EDinosaurMood, OldMood, EDinosaurMood, NewMood);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDinosaurActivityChanged, EDinosaurActivity, OldActivity, EDinosaurActivity, NewActivity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDinosaurMemoryUpdated, AActor*, Actor, float, EmotionalValue, int32, EncounterCount);

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
    // Species and Individual Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    FDataTableRowHandle SpeciesDataHandle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Individual")
    FDinosaurPersonality Personality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Individual")
    FDinosaurDailySchedule DailySchedule;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    EDinosaurMood CurrentMood = EDinosaurMood::Calm;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    EDinosaurActivity CurrentActivity = EDinosaurActivity::Resting;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    float CurrentTime = 0.0f; // Game time in hours (0-24)

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    AActor* CurrentTarget = nullptr;

    // Memory System
    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    TArray<FDinosaurMemoryEntry> MemoryEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxMemoryEntries = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MemoryDecayRate = 0.1f; // Per hour

    // Domestication System
    UPROPERTY(BlueprintReadOnly, Category = "Domestication")
    float DomesticationLevel = 0.0f; // 0.0 to 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domestication")
    float DomesticationGainRate = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domestication")
    float DomesticationLossRate = 0.005f;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnDinosaurMoodChanged OnMoodChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnDinosaurActivityChanged OnActivityChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnDinosaurMemoryUpdated OnMemoryUpdated;

    // Public Interface
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetMood(EDinosaurMood NewMood);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetActivity(EDinosaurActivity NewActivity);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void RememberActor(AActor* Actor, float EmotionalValue);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    FDinosaurMemoryEntry* GetMemoryOfActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void ForgetActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Domestication")
    void ModifyDomestication(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Domestication")
    bool IsDomesticated() const { return DomesticationLevel >= 0.7f; }

    UFUNCTION(BlueprintCallable, Category = "Domestication")
    bool CanBeDomesticated() const;

    UFUNCTION(BlueprintCallable, Category = "Species")
    FDinosaurSpeciesData* GetSpeciesData() const;

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    EDinosaurActivity GetScheduledActivity() const;

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    bool ShouldBeActive() const;

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    FVector GetPreferredLocation() const;

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    float GetPersonalityModifier(const FString& TraitName) const;

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void ReactToPlayer(AActor* Player, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void ReactToThreat(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void ReactToFood(AActor* Food);

private:
    void UpdateMemory(float DeltaTime);
    void UpdateDomestication(float DeltaTime);
    void UpdateDailyRoutine();
    void GeneratePersonalityName();
    void InitializeDailySchedule();
    
    float LastMemoryUpdate = 0.0f;
    float LastDomesticationUpdate = 0.0f;
};