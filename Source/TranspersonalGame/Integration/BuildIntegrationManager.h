#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "../SharedTypes.h"
#include "BuildIntegrationManager.generated.h"

UENUM(BlueprintType)
enum class EBuild_BuildStatus : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    Initializing    UMETA(DisplayName = "Initializing"),
    Compiling       UMETA(DisplayName = "Compiling"),
    Testing         UMETA(DisplayName = "Testing"),
    Success         UMETA(DisplayName = "Success"),
    Failed          UMETA(DisplayName = "Failed"),
    Warning         UMETA(DisplayName = "Warning")
};

USTRUCT(BlueprintType)
struct FBuild_ModuleTestResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    int32 ClassesLoaded;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    int32 TotalClasses;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    EBuild_BuildStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TArray<FString> ErrorMessages;

    FBuild_ModuleTestResult()
    {
        ModuleName = TEXT("");
        ClassesLoaded = 0;
        TotalClasses = 0;
        Status = EBuild_BuildStatus::Unknown;
    }
};

USTRUCT(BlueprintType)
struct FBuild_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    FString CycleId;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    FDateTime Timestamp;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    EBuild_BuildStatus OverallStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TArray<FBuild_ModuleTestResult> ModuleResults;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TArray<FString> CompilationErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TArray<FString> Recommendations;

    FBuild_IntegrationReport()
    {
        CycleId = TEXT("");
        Timestamp = FDateTime::Now();
        OverallStatus = EBuild_BuildStatus::Unknown;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABuild_IntegrationMonitor : public AActor
{
    GENERATED_BODY()

public:
    ABuild_IntegrationMonitor();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* StatusMesh;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    EBuild_BuildStatus CurrentStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    FBuild_IntegrationReport LastReport;

public:
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void UpdateBuildStatus(EBuild_BuildStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void SetIntegrationReport(const FBuild_IntegrationReport& Report);

    UFUNCTION(BlueprintPure, Category = "Build Integration")
    EBuild_BuildStatus GetCurrentStatus() const { return CurrentStatus; }

    UFUNCTION(BlueprintPure, Category = "Build Integration")
    FBuild_IntegrationReport GetLastReport() const { return LastReport; }

private:
    void UpdateVisualStatus();
};

UCLASS()
class TRANSPERSONALGAME_API UBuild_IntegrationSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void RunIntegrationTests();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void TestModuleLoading();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void GenerateIntegrationReport();

    UFUNCTION(BlueprintPure, Category = "Build Integration")
    FBuild_IntegrationReport GetCurrentReport() const { return CurrentReport; }

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool SaveReportToFile(const FString& FilePath);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool LoadReportFromFile(const FString& FilePath);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    FBuild_IntegrationReport CurrentReport;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TArray<FString> ModulesToTest;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TArray<FString> CriticalClasses;

private:
    void InitializeTestParameters();
    FBuild_ModuleTestResult TestModule(const FString& ModuleName, const TArray<FString>& ClassPaths);
    bool TestClassLoading(const FString& ClassPath, FString& OutError);
    void UpdateOverallStatus();
};