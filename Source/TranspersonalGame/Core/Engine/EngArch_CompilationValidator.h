#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "EngArch_CompilationValidator.generated.h"

UENUM(BlueprintType)
enum class EEng_CompilationStatus : uint8
{
    Unknown,
    Compiling,
    Success,
    Failed,
    Warning
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_CompilationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    EEng_CompilationStatus Status = EEng_CompilationStatus::Unknown;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 ErrorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 WarningCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    TArray<FString> ErrorMessages;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    float CompilationTime = 0.0f;

    FEng_CompilationResult()
    {
        Status = EEng_CompilationStatus::Unknown;
        ModuleName = TEXT("");
        ErrorCount = 0;
        WarningCount = 0;
        CompilationTime = 0.0f;
    }
};

/**
 * Engine Architect Compilation Validator
 * Validates C++ compilation status and manages build integrity
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEngArch_CompilationValidator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEngArch_CompilationValidator();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Compilation validation
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FEng_CompilationResult ValidateModuleCompilation(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsProjectCompiled() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FEng_CompilationResult> GetAllModuleStatus();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void RefreshCompilationStatus();

    // Build management
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool TriggerHotReload();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void LogCompilationReport();

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCompilationComplete, FEng_CompilationResult, Result);
    UPROPERTY(BlueprintAssignable, Category = "Engine Architecture")
    FOnCompilationComplete OnCompilationComplete;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    TArray<FEng_CompilationResult> ModuleResults;

    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    bool bIsValidatingCompilation = false;

    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    float LastValidationTime = 0.0f;

private:
    void ValidateAllModules();
    FEng_CompilationResult ValidateSpecificModule(const FString& ModuleName);
    void BroadcastCompilationResult(const FEng_CompilationResult& Result);
};