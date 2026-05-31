#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Eng_CompilationManager.generated.h"

UENUM(BlueprintType)
enum class EEng_CompilationStatus : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    Compiling       UMETA(DisplayName = "Compiling"),
    Success         UMETA(DisplayName = "Success"),
    Failed          UMETA(DisplayName = "Failed"),
    Disabled        UMETA(DisplayName = "Disabled")
};

USTRUCT(BlueprintType)
struct FEng_ModuleCompilationInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    EEng_CompilationStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 ErrorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 WarningCount;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    TArray<FString> ErrorMessages;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    float LastCompileTime;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    bool bIsEnabled;

    FEng_ModuleCompilationInfo()
    {
        ModuleName = TEXT("");
        Status = EEng_CompilationStatus::Unknown;
        ErrorCount = 0;
        WarningCount = 0;
        LastCompileTime = 0.0f;
        bIsEnabled = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_CompilationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool CheckModuleCompilationStatus(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    TArray<FEng_ModuleCompilationInfo> GetAllModuleStatus();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool EnableModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool DisableModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    int32 GetTotalEnabledModules() const;

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    int32 GetTotalDisabledModules() const;

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool ValidateMinimalBuild();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    TArray<FString> GetCriticalModules() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    TArray<FEng_ModuleCompilationInfo> ModuleInfos;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    TArray<FString> CriticalModules;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 TotalSourceFiles;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 EnabledSourceFiles;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 DisabledSourceFiles;

private:
    void InitializeModuleTracking();
    void UpdateModuleStatus(const FString& ModuleName, EEng_CompilationStatus Status);
    bool IsModuleFileEnabled(const FString& FilePath);
    void ScanSourceDirectory();
};