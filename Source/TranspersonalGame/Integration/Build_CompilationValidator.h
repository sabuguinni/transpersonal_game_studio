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
    TArray<FString> ErrorMessages;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> WarningMessages;

    UPROPERTY(BlueprintReadOnly)
    float CompilationTime;

    FBuild_ModuleStatus()
    {
        ModuleName = TEXT("");
        Status = EBuild_CompilationStatus::Unknown;
        CompilationTime = 0.0f;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UBuild_CompilationValidator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_CompilationValidator();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ValidateAllModules();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ValidateModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    EBuild_CompilationStatus GetModuleStatus(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FBuild_ModuleStatus> GetAllModuleStatuses();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool AreAllModulesValid();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void GenerateCompilationReport();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void CheckForDuplicateTypes();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ValidateHeaderIncludes();

protected:
    UPROPERTY(BlueprintReadOnly)
    TMap<FString, FBuild_ModuleStatus> ModuleStatuses;

    UPROPERTY(BlueprintReadOnly)
    bool bValidationInProgress;

    UPROPERTY(BlueprintReadOnly)
    float LastValidationTime;

private:
    void CheckModuleCompilation(const FString& ModuleName);
    void ParseCompilationErrors(const FString& ModuleName, const FString& LogOutput);
    void CheckForMissingImplementations();
    void ValidateUPropertyMacros();
    void CheckGeneratedHeaders();
};