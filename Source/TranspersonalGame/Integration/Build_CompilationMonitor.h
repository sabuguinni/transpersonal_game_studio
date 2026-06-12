#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "Build_CompilationMonitor.generated.h"

UENUM(BlueprintType)
enum class EBuild_CompilationStatus : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    Compiling       UMETA(DisplayName = "Compiling"),
    Success         UMETA(DisplayName = "Success"),
    Failed          UMETA(DisplayName = "Failed"),
    Warning         UMETA(DisplayName = "Warning")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_CompilationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    EBuild_CompilationStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 ErrorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 WarningCount;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    float CompilationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FDateTime Timestamp;

    FBuild_CompilationResult()
    {
        ModuleName = TEXT("");
        Status = EBuild_CompilationStatus::Unknown;
        ErrorMessage = TEXT("");
        ErrorCount = 0;
        WarningCount = 0;
        CompilationTime = 0.0f;
        Timestamp = FDateTime::Now();
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCompilationComplete, const FBuild_CompilationResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCompilationStarted, const FString&, ModuleName);

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UBuild_CompilationMonitor : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    void StartMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    void StopMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool IsMonitoring() const;

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    TArray<FBuild_CompilationResult> GetCompilationHistory() const;

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    FBuild_CompilationResult GetLastCompilationResult() const;

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    void ClearCompilationHistory();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    void TriggerManualCompilation();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool ValidateModuleIntegrity(const FString& ModuleName);

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnCompilationComplete OnCompilationComplete;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnCompilationStarted OnCompilationStarted;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Monitoring")
    TArray<FBuild_CompilationResult> CompilationHistory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bAutoMonitoring;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MonitoringInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    int32 MaxHistoryEntries;

private:
    bool bIsMonitoring;
    FTimerHandle MonitoringTimerHandle;
    
    void PerformMonitoringCheck();
    void ProcessCompilationResult(const FBuild_CompilationResult& Result);
    FBuild_CompilationResult AnalyzeCurrentBuildState();
    void CheckModuleStatus();
    void LogCompilationEvent(const FBuild_CompilationResult& Result);
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABuild_CompilationTestActor : public AActor
{
    GENERATED_BODY()

public:
    ABuild_CompilationTestActor();

    UFUNCTION(BlueprintCallable, Category = "Testing")
    void RunCompilationTests();

    UFUNCTION(BlueprintCallable, Category = "Testing")
    void TestModuleLoading();

    UFUNCTION(BlueprintCallable, Category = "Testing")
    void TestClassInstantiation();

    UFUNCTION(BlueprintCallable, Category = "Testing")
    void GenerateCompilationReport();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testing")
    TArray<FString> TestModules;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testing")
    TArray<FString> TestClasses;

    UPROPERTY(BlueprintReadOnly, Category = "Results")
    TArray<FBuild_CompilationResult> TestResults;

    virtual void BeginPlay() override;

private:
    void InitializeTestData();
    FBuild_CompilationResult TestSingleModule(const FString& ModuleName);
    FBuild_CompilationResult TestSingleClass(const FString& ClassName);
};