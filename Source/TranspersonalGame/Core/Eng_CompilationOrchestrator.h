#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Eng_CompilationOrchestrator.generated.h"

UENUM(BlueprintType)
enum class EEng_CompilationStatus : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    Compiling       UMETA(DisplayName = "Compiling"),
    Success         UMETA(DisplayName = "Success"),
    Failed          UMETA(DisplayName = "Failed"),
    Critical        UMETA(DisplayName = "Critical Error")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ModuleStatus
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
    FString LastError;

    FEng_ModuleStatus()
    {
        ModuleName = TEXT("");
        Status = EEng_CompilationStatus::Unknown;
        ErrorCount = 0;
        WarningCount = 0;
        LastError = TEXT("");
    }
};

/**
 * Engine Architect - Compilation Orchestrator
 * Manages compilation status across all modules and ensures clean builds
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_CompilationOrchestrator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_CompilationOrchestrator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void ValidateAllModules();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsProjectCompilationClean() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void ForceRecompileModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FEng_ModuleStatus> GetModuleStatusList() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void CleanOrphanedHeaders();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void ValidateCriticalClasses();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    TMap<FString, FEng_ModuleStatus> ModuleStatusMap;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    bool bIsCompilationClean;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    float LastValidationTime;

private:
    void CheckCoreClasses();
    void CheckPhysicsClasses();
    void CheckBiomeClasses();
    void CheckCharacterClasses();
    void CheckDinosaurClasses();
    
    void LogCompilationStatus(const FString& ModuleName, EEng_CompilationStatus Status, const FString& Details = TEXT(""));
};