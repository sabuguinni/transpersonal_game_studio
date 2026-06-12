#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Build_CompilationValidator.generated.h"

UENUM(BlueprintType)
enum class EBuild_CompilationStatus : uint8
{
    Unknown,
    Compiling,
    Success,
    Failed,
    Warning
};

USTRUCT(BlueprintType)
struct FBuild_ModuleStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly)
    EBuild_CompilationStatus Status;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> Errors;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> Warnings;

    UPROPERTY(BlueprintReadOnly)
    float CompileTime;

    FBuild_ModuleStatus()
    {
        ModuleName = TEXT("");
        Status = EBuild_CompilationStatus::Unknown;
        CompileTime = 0.0f;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UBuild_CompilationValidator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ValidateAllModules();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool CheckModuleCompilation(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FBuild_ModuleStatus> GetModuleStatuses() const { return ModuleStatuses; }

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    EBuild_CompilationStatus GetOverallStatus() const { return OverallStatus; }

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void GenerateCompilationReport();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TArray<FBuild_ModuleStatus> ModuleStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    EBuild_CompilationStatus OverallStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    FDateTime LastValidationTime;

private:
    void ValidateTranspersonalGameModule();
    void ValidateEngineModules();
    void CheckHeaderIncludes();
    void CheckLinkerDependencies();
    void UpdateOverallStatus();
};