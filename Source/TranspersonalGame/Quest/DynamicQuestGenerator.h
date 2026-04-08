#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "../CrowdSimulation/MassDinosaurFragments.h"
#include "DynamicQuestGenerator.generated.h"

class UMassEntitySubsystem;
class UQuestSystem;

USTRUCT(BlueprintType)
struct FEcosystemEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTag EventType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Radius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMassEntityHandle> InvolvedEntities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Intensity = 1.0f; // 0-1, quão significativo é o evento

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTagContainer ContextTags;
};

USTRUCT(BlueprintType)
struct FQuestTemplate
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName TemplateID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText QuestNameTemplate; // "Observe {DinosaurSpecies} hunting behavior"

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText QuestDescriptionTemplate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EQuestType QuestType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> RequiredVariables; // {DinosaurSpecies}, {Location}, etc.

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTagContainer TriggerTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CooldownTime = 300.0f; // Tempo antes de poder gerar novamente

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxActiveInstances = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Priority = 1.0f;
};

USTRUCT(BlueprintType)
struct FDinosaurBehaviorPattern
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTag SpeciesTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PatternRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EBehaviorState DominantBehavior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMassEntityHandle> ParticipatingEntities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PatternStrength = 1.0f; // Quão definido é o padrão

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration = 0.0f; // Há quanto tempo este padrão existe

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsInteresting = false; // Se vale a pena gerar uma quest
};

/**
 * Gerador dinâmico de missões que observa o ecossistema Mass e cria
 * experiências emergentes baseadas no comportamento real dos dinossauros
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UDynamicQuestGenerator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Ecosystem Monitoring
    UFUNCTION(BlueprintCallable, Category = "Dynamic Quest")
    void MonitorEcosystem();

    UFUNCTION(BlueprintCallable, Category = "Dynamic Quest")
    void RegisterEcosystemEvent(const FEcosystemEvent& Event);

    UFUNCTION(BlueprintCallable, Category = "Dynamic Quest")
    TArray<FDinosaurBehaviorPattern> AnalyzeBehaviorPatterns();

    // Quest Generation
    UFUNCTION(BlueprintCallable, Category = "Dynamic Quest")
    FName GenerateObservationQuest(const FDinosaurBehaviorPattern& Pattern);

    UFUNCTION(BlueprintCallable, Category = "Dynamic Quest")
    FName GenerateMigrationQuest(const TArray<FMassEntityHandle>& MigratingHerd);

    UFUNCTION(BlueprintCallable, Category = "Dynamic Quest")
    FName GenerateHuntingQuest(const FEcosystemEvent& HuntingEvent);

    UFUNCTION(BlueprintCallable, Category = "Dynamic Quest")
    FName GenerateNestingQuest(FVector NestLocation, FGameplayTag SpeciesTag);

    UFUNCTION(BlueprintCallable, Category = "Dynamic Quest")
    FName GenerateEmergencyQuest(const FEcosystemEvent& EmergencyEvent);

    // Pattern Recognition
    UFUNCTION(BlueprintCallable, Category = "Dynamic Quest")
    bool DetectHerdMigration(TArray<FMassEntityHandle>& OutHerd, FVector& OutDestination);

    UFUNCTION(BlueprintCallable, Category = "Dynamic Quest")
    bool DetectPredatorHunt(FMassEntityHandle& OutPredator, TArray<FMassEntityHandle>& OutPrey);

    UFUNCTION(BlueprintCallable, Category = "Dynamic Quest")
    bool DetectNestingBehavior(FVector& OutNestLocation, FGameplayTag& OutSpecies);

    UFUNCTION(BlueprintCallable, Category = "Dynamic Quest")
    bool DetectTerritorialDispute(TArray<FMassEntityHandle>& OutCompetitors, FVector& OutTerritory);

    // Template System
    UFUNCTION(BlueprintCallable, Category = "Dynamic Quest")
    void LoadQuestTemplates(class UDataTable* TemplateTable);

    UFUNCTION(BlueprintCallable, Category = "Dynamic Quest")
    FName InstantiateQuestFromTemplate(FName TemplateID, const TMap<FString, FString>& Variables);

    // Player Context
    UFUNCTION(BlueprintCallable, Category = "Dynamic Quest")
    void UpdatePlayerContext(FVector PlayerLocation, const FGameplayTagContainer& PlayerTags);

    UFUNCTION(BlueprintCallable, Category = "Dynamic Quest")
    float CalculateQuestRelevance(const FEcosystemEvent& Event, FVector PlayerLocation);

protected:
    UPROPERTY()
    TObjectPtr<UMassEntitySubsystem> MassEntitySubsystem;

    UPROPERTY()
    TObjectPtr<UQuestSystem> QuestSystem;

    UPROPERTY()
    TArray<FEcosystemEvent> RecentEvents;

    UPROPERTY()
    TArray<FDinosaurBehaviorPattern> ActivePatterns;

    UPROPERTY()
    TMap<FName, FQuestTemplate> QuestTemplates;

    UPROPERTY()
    TMap<FName, float> TemplateCooldowns;

    // Player Context
    UPROPERTY()
    FVector LastPlayerLocation;

    UPROPERTY()
    FGameplayTagContainer PlayerContextTags;

    UPROPERTY()
    float LastPlayerUpdate = 0.0f;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float EcosystemMonitoringRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float EventRelevanceThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    int32 MaxActiveEvents = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float PatternAnalysisInterval = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    int32 MaxConcurrentDynamicQuests = 3;

private:
    FTimerHandle EcosystemMonitorTimer;
    FTimerHandle PatternAnalysisTimer;

    void AnalyzeEntityClusters();
    void DetectEmergentBehaviors();
    void CleanupOldEvents();
    void UpdateTemplateCooldowns();
    
    FString ProcessQuestTemplate(const FString& Template, const TMap<FString, FString>& Variables);
    bool IsLocationSafeForPlayer(FVector Location, float SafetyRadius = 500.0f);
    FGameplayTag GetDominantSpeciesInArea(FVector Center, float Radius);
};