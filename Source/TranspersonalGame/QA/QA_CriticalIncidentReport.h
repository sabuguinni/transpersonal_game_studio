#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "QA_CriticalIncidentReport.generated.h"

/**
 * QA Critical Incident Report System
 * Tracks and reports critical failures in the development pipeline
 * Agent #18 - QA & Testing Agent
 */

UENUM(BlueprintType)
enum class EQA_IncidentSeverity : uint8
{
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"), 
    High        UMETA(DisplayName = "High"),
    Critical    UMETA(DisplayName = "Critical"),
    Blocker     UMETA(DisplayName = "Blocker")
};

UENUM(BlueprintType)
enum class EQA_IncidentCategory : uint8
{
    Compilation     UMETA(DisplayName = "Compilation"),
    Runtime         UMETA(DisplayName = "Runtime"),
    Performance     UMETA(DisplayName = "Performance"),
    VFX             UMETA(DisplayName = "VFX"),
    Audio           UMETA(DisplayName = "Audio"),
    Networking      UMETA(DisplayName = "Networking"),
    Bridge          UMETA(DisplayName = "UE5 Bridge"),
    Integration     UMETA(DisplayName = "Integration")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_IncidentReport
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Incident")
    FString IncidentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Incident")
    EQA_IncidentSeverity Severity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Incident")
    EQA_IncidentCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Incident")
    FString Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Incident")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Incident")
    FString AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Incident")
    FString CycleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Incident")
    FDateTime Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Incident")
    TArray<FString> AffectedSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Incident")
    FString ReproductionSteps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Incident")
    FString WorkaroundSolution;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Incident")
    bool bResolved;

    FQA_IncidentReport()
    {
        Severity = EQA_IncidentSeverity::Medium;
        Category = EQA_IncidentCategory::Runtime;
        Timestamp = FDateTime::Now();
        bResolved = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_CriticalIncidentReport : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQA_CriticalIncidentReport();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "QA Incident")
    FString ReportIncident(const FQA_IncidentReport& Incident);

    UFUNCTION(BlueprintCallable, Category = "QA Incident")
    void ResolveIncident(const FString& IncidentID, const FString& Resolution);

    UFUNCTION(BlueprintCallable, Category = "QA Incident")
    TArray<FQA_IncidentReport> GetActiveIncidents();

    UFUNCTION(BlueprintCallable, Category = "QA Incident")
    TArray<FQA_IncidentReport> GetIncidentsByCategory(EQA_IncidentCategory Category);

    UFUNCTION(BlueprintCallable, Category = "QA Incident")
    TArray<FQA_IncidentReport> GetIncidentsBySeverity(EQA_IncidentSeverity Severity);

    UFUNCTION(BlueprintCallable, Category = "QA Incident")
    void ReportBridgeFailure(const FString& AgentID, const FString& CycleID, const FString& ErrorDetails);

    UFUNCTION(BlueprintCallable, Category = "QA Incident")
    void ReportVFXSystemFailure(const FString& SystemName, const FString& ErrorDetails);

    UFUNCTION(BlueprintCallable, Category = "QA Incident")
    void ReportCompilationFailure(const FString& ModuleName, const FString& ErrorDetails);

    UFUNCTION(BlueprintCallable, Category = "QA Incident")
    bool IsSystemBlocked(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "QA Incident")
    void GenerateIncidentReport();

    UFUNCTION(BlueprintCallable, Category = "QA Incident")
    void ExportIncidentsToFile(const FString& FilePath);

protected:
    UPROPERTY()
    TArray<FQA_IncidentReport> IncidentDatabase;

    UPROPERTY()
    TMap<FString, bool> SystemBlockStatus;

    UPROPERTY()
    int32 NextIncidentID;

private:
    FString GenerateIncidentID();
    void LogIncidentToConsole(const FQA_IncidentReport& Incident);
    void UpdateSystemBlockStatus();
};