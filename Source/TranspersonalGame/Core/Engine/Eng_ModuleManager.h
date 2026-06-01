#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Modules/ModuleManager.h"
#include "Eng_ModuleManager.generated.h"

UENUM(BlueprintType)
enum class EEng_ModuleType : uint8
{
    Core = 0,
    Gameplay = 1,
    Rendering = 2,
    Audio = 3,
    AI = 4,
    Physics = 5,
    Networking = 6,
    UI = 7,
    Tools = 8,
    Plugin = 9
};

UENUM(BlueprintType)
enum class EEng_ModuleStatus : uint8
{
    NotLoaded = 0,
    Loading = 1,
    Loaded = 2,
    Failed = 3,
    Unloading = 4
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ModuleInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    EEng_ModuleType ModuleType;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    EEng_ModuleStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    FString Version;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    TArray<FString> Dependencies;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    bool bIsGameModule;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    bool bIsEditorModule;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    float LoadTime;

    FEng_ModuleInfo()
    {
        ModuleName = TEXT("");
        ModuleType = EEng_ModuleType::Core;
        Status = EEng_ModuleStatus::NotLoaded;
        Version = TEXT("1.0.0");
        bIsGameModule = true;
        bIsEditorModule = false;
        LoadTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_CompilationInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    bool bCompilationSuccessful;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 ErrorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 WarningCount;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    TArray<FString> ErrorMessages;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    TArray<FString> WarningMessages;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    float CompilationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FDateTime LastCompileTime;

    FEng_CompilationInfo()
    {
        bCompilationSuccessful = false;
        ErrorCount = 0;
        WarningCount = 0;
        CompilationTime = 0.0f;
        LastCompileTime = FDateTime::Now();
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_ModuleManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_ModuleManager();

    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Module Management
    UFUNCTION(BlueprintCallable, Category = "Module Management")
    void RegisterModule(const FString& ModuleName, EEng_ModuleType ModuleType, const FString& Version = TEXT("1.0.0"));

    UFUNCTION(BlueprintCallable, Category = "Module Management")
    bool IsModuleLoaded(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Module Management")
    EEng_ModuleStatus GetModuleStatus(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Module Management")
    TArray<FString> GetLoadedModules() const;

    UFUNCTION(BlueprintCallable, Category = "Module Management")
    TArray<FEng_ModuleInfo> GetAllModuleInfo() const;

    // Module Dependencies
    UFUNCTION(BlueprintCallable, Category = "Module Management")
    void AddModuleDependency(const FString& ModuleName, const FString& DependsOn);

    UFUNCTION(BlueprintCallable, Category = "Module Management")
    TArray<FString> GetModuleDependencies(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Module Management")
    bool ValidateModuleDependencies() const;

    // Compilation Status
    UFUNCTION(BlueprintCallable, Category = "Compilation")
    FEng_CompilationInfo GetCompilationStatus() const;

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    void UpdateCompilationStatus(bool bSuccess, int32 Errors, int32 Warnings, float CompileTime);

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    void AddCompilationError(const FString& ErrorMessage);

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    void AddCompilationWarning(const FString& WarningMessage);

    // Module Loading
    UFUNCTION(BlueprintCallable, Category = "Module Management")
    bool LoadModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Module Management")
    bool UnloadModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Module Management")
    void ReloadModule(const FString& ModuleName);

    // Validation
    UFUNCTION(BlueprintCallable, Category = "Module Management")
    bool ValidateModuleIntegrity(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Module Management")
    TArray<FString> GetModuleErrors() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Module Management")
    TMap<FString, FEng_ModuleInfo> RegisteredModules;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FEng_CompilationInfo CurrentCompilationInfo;

    UPROPERTY(BlueprintReadOnly, Category = "Module Management")
    TArray<FString> ModuleErrors;

private:
    void InitializeGameModules();
    void ScanForModules();
    bool CheckModuleDependencies(const FString& ModuleName, TSet<FString>& Visited) const;
    void UpdateModuleStatus(const FString& ModuleName, EEng_ModuleStatus NewStatus);
};