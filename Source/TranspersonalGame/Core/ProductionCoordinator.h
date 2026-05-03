#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "ProductionCoordinator.generated.h"

/**
 * Coordenador de produção que monitoriza o progresso do Milestone 1
 * e coordena tarefas entre os 18 agentes especializados
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UProductionCoordinator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UProductionCoordinator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Milestone 1 tracking
    UFUNCTION(BlueprintCallable, Category = "Production")
    bool IsMilestone1Complete() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    float GetMilestone1Progress() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateTaskProgress(const FString& AgentName, const FString& TaskName, bool bCompleted);

    // Agent coordination
    UFUNCTION(BlueprintCallable, Category = "Production")
    void RegisterAgent(const FString& AgentName, int32 Priority);

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FString> GetActiveAgents() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool CanAgentProceed(const FString& AgentName) const;

    // Critical state management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void ReportCriticalIssue(const FString& AgentName, const FString& Issue);

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FString> GetCriticalIssues() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void ClearCriticalIssue(const FString& Issue);

    // Biome coordination
    UFUNCTION(BlueprintCallable, Category = "Production")
    FVector GetBiomeCenter(EDir_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    FVector GetRandomLocationInBiome(EDir_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool IsLocationInBiome(const FVector& Location, EDir_BiomeType BiomeType) const;

protected:
    // Milestone 1 tasks tracking
    UPROPERTY(BlueprintReadOnly, Category = "Production")
    TMap<FString, bool> Milestone1Tasks;

    // Agent registry
    UPROPERTY(BlueprintReadOnly, Category = "Production")
    TMap<FString, int32> RegisteredAgents;

    // Critical issues
    UPROPERTY(BlueprintReadOnly, Category = "Production")
    TArray<FString> CriticalIssues;

    // Biome definitions
    UPROPERTY(BlueprintReadOnly, Category = "Production")
    TMap<EDir_BiomeType, FDir_BiomeData> BiomeData;

private:
    void InitializeMilestone1Tasks();
    void InitializeBiomeData();
    void ValidateMapState();
};