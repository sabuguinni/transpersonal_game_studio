#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DinosaurBehaviorTypes.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "DinosaurBehaviorComponent.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;
class UBehaviorTree;

/**
 * Core component that manages dinosaur behavior, needs, memory, and daily routines
 * Each dinosaur has its own independent life cycle and decision-making process
 */
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
    // Core Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    EDinosaurSpecies Species = EDinosaurSpecies::Compsognathus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    TSoftObjectPtr<UDataTable> SpeciesDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    EDinosaurPersonality PrimaryPersonality = EDinosaurPersonality::Curious;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    EDinosaurPersonality SecondaryPersonality = EDinosaurPersonality::Timid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    FDinosaurPhysicalTraits PhysicalTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    EDinosaurBehaviorState CurrentBehaviorState = EDinosaurBehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs")
    FDinosaurNeeds CurrentNeeds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FDinosaurMemory Memory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domestication")
    EDomesticationStage DomesticationStage = EDomesticationStage::Wild;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domestication")
    float DomesticationProgress = 0.0f;

    // Unique Identity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FString DinosaurName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    int32 UniqueID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    float Age = 1.0f; // In years

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    bool bIsMale = true;

    // Behavior Tree Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    TSoftObjectPtr<UBehaviorTree> BehaviorTreeAsset;

    // Daily Routine
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    bool bFollowsDailyRoutine = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    float CurrentGameTimeHour = 6.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    float LastRoutineUpdateTime = 0.0f;

    // Social Behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    TArray<TWeakObjectPtr<UDinosaurBehaviorComponent>> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    TWeakObjectPtr<UDinosaurBehaviorComponent> PackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    bool bIsPackLeader = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float SocialInteractionCooldown = 0.0f;

    // Territory
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    FVector TerritoryCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float TerritoryRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    bool bHasTerritory = false;

    // Player Interaction
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Interaction")
    float PlayerDetectionRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Interaction")
    float PlayerTrustLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Interaction")
    float LastPlayerInteractionTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Interaction")
    bool bCanSeePlayer = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Interaction")
    TWeakObjectPtr<APawn> LastKnownPlayerLocation;

public:
    // Core Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetBehaviorState(EDinosaurBehaviorState NewState);

    UFUNCTION(BlueprintPure, Category = "Behavior")
    EDinosaurBehaviorState GetBehaviorState() const { return CurrentBehaviorState; }

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    bool CanTransitionToBehaviorState(EDinosaurBehaviorState TargetState) const;

    // Needs Management
    UFUNCTION(BlueprintCallable, Category = "Needs")
    void UpdateNeeds(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Needs")
    void SatisfyNeed(const FString& NeedType, float Amount);

    UFUNCTION(BlueprintPure, Category = "Needs")
    float GetMostUrgentNeedValue() const;

    UFUNCTION(BlueprintPure, Category = "Needs")
    FString GetMostUrgentNeedType() const;

    // Memory Functions
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void RememberLocation(FVector Location, const FString& LocationType);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void RememberEntity(AActor* Entity, const FString& EntityType);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    FVector GetNearestRememberedLocation(const FString& LocationType) const;

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void ForgetOldMemories(float MaxAge = 3600.0f); // 1 hour default

    // Player Interaction
    UFUNCTION(BlueprintCallable, Category = "Player Interaction")
    void RegisterPlayerInteraction(bool bPositive, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Player Interaction")
    void UpdatePlayerTrust(float TrustChange);

    UFUNCTION(BlueprintPure, Category = "Player Interaction")
    bool CanPlayerApproach() const;

    UFUNCTION(BlueprintPure, Category = "Player Interaction")
    float GetPlayerTrustLevel() const { return PlayerTrustLevel; }

    // Domestication
    UFUNCTION(BlueprintCallable, Category = "Domestication")
    void AdvanceDomestication(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Domestication")
    void RegressDomestication(float Amount);

    UFUNCTION(BlueprintPure, Category = "Domestication")
    bool CanBeDomesticated() const;

    UFUNCTION(BlueprintPure, Category = "Domestication")
    float GetDomesticationTimeRequired() const;

    // Social Behavior
    UFUNCTION(BlueprintCallable, Category = "Social")
    void JoinPack(UDinosaurBehaviorComponent* NewPackMember);

    UFUNCTION(BlueprintCallable, Category = "Social")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Social")
    void SetPackLeader(UDinosaurBehaviorComponent* NewLeader);

    UFUNCTION(BlueprintPure, Category = "Social")
    bool IsInPack() const;

    UFUNCTION(BlueprintPure, Category = "Social")
    int32 GetPackSize() const;

    // Daily Routine
    UFUNCTION(BlueprintCallable, Category = "Routine")
    void UpdateDailyRoutine(float CurrentTime);

    UFUNCTION(BlueprintPure, Category = "Routine")
    EDinosaurBehaviorState GetPreferredBehaviorForTime(float TimeOfDay) const;

    UFUNCTION(BlueprintCallable, Category = "Routine")
    void SetCustomRoutine(const TArray<EDinosaurBehaviorState>& CustomStates, const TArray<float>& CustomTimes);

    // Territory
    UFUNCTION(BlueprintCallable, Category = "Territory")
    void EstablishTerritory(FVector Center, float Radius);

    UFUNCTION(BlueprintPure, Category = "Territory")
    bool IsInTerritory(FVector Location) const;

    UFUNCTION(BlueprintPure, Category = "Territory")
    bool IsLocationInTerritory(FVector Location) const;

    // Species Data
    UFUNCTION(BlueprintPure, Category = "Species")
    FDinosaurSpeciesData GetSpeciesData() const;

    UFUNCTION(BlueprintCallable, Category = "Species")
    void InitializeFromSpeciesData();

    // Utility Functions
    UFUNCTION(BlueprintPure, Category = "Utility")
    FString GetDinosaurDisplayName() const;

    UFUNCTION(BlueprintPure, Category = "Utility")
    FString GetBehaviorStateDisplayName() const;

    UFUNCTION(BlueprintPure, Category = "Utility")
    FLinearColor GetDinosaurPrimaryColor() const { return PhysicalTraits.PrimaryColor; }

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void GenerateRandomPhysicalTraits();

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void GenerateRandomPersonality();

    // Debug Functions
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor = true)
    void DebugPrintBehaviorInfo() const;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawDebugInfo(float Duration = 5.0f) const;

protected:
    // Internal update functions
    void UpdateNeedsDecay(float DeltaTime);
    void UpdateBehaviorStateLogic(float DeltaTime);
    void UpdatePlayerDetection();
    void UpdateSocialBehavior();
    void ProcessEnvironmentalStimuli();

    // Behavior state transition logic
    bool ShouldTransitionToFeeding() const;
    bool ShouldTransitionToDrinking() const;
    bool ShouldTransitionToResting() const;
    bool ShouldTransitionToSocializing() const;
    bool ShouldTransitionToAlert() const;
    bool ShouldTransitionToFleeing() const;

    // Helper functions
    float CalculateNeedUrgency(float NeedValue, float CriticalThreshold = 30.0f) const;
    FVector FindNearestResourceLocation(const FString& ResourceType) const;
    bool IsPlayerNearby() const;
    bool IsPlayerThreatening() const;

private:
    // Cached species data
    UPROPERTY()
    FDinosaurSpeciesData CachedSpeciesData;

    // Internal timers
    float NeedsUpdateTimer = 0.0f;
    float MemoryCleanupTimer = 0.0f;
    float BehaviorUpdateTimer = 0.0f;
    float PlayerDetectionTimer = 0.0f;

    // Update intervals
    static constexpr float NEEDS_UPDATE_INTERVAL = 1.0f;
    static constexpr float MEMORY_CLEANUP_INTERVAL = 60.0f;
    static constexpr float BEHAVIOR_UPDATE_INTERVAL = 0.5f;
    static constexpr float PLAYER_DETECTION_INTERVAL = 0.25f;
};