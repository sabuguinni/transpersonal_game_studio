#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Eng_ArchitectureValidator.generated.h"

UENUM(BlueprintType)
enum class EEng_ValidationLevel : uint8
{
    Critical    UMETA(DisplayName = "Critical"),
    High        UMETA(DisplayName = "High"),
    Medium      UMETA(DisplayName = "Medium"),
    Low         UMETA(DisplayName = "Low")
};

UENUM(BlueprintType)
enum class EEng_ModuleType : uint8
{
    Core        UMETA(DisplayName = "Core"),
    Gameplay    UMETA(DisplayName = "Gameplay"),
    World       UMETA(DisplayName = "World"),
    Character   UMETA(DisplayName = "Character"),
    AI          UMETA(DisplayName = "AI"),
    Audio       UMETA(DisplayName = "Audio"),
    VFX         UMETA(DisplayName = "VFX"),
    UI          UMETA(DisplayName = "UI")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ValidationRule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    FString RuleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    EEng_ValidationLevel Level;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    bool bIsActive;

    FEng_ValidationRule()
    {
        RuleName = TEXT("");
        Description = TEXT("");
        Level = EEng_ValidationLevel::Medium;
        bIsActive = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ModuleInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    FString ModuleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    EEng_ModuleType ModuleType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    int32 Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    bool bIsCompiled;

    FEng_ModuleInfo()
    {
        ModuleName = TEXT("");
        ModuleType = EEng_ModuleType::Core;
        Priority = 0;
        bIsCompiled = false;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_ArchitectureValidator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Architecture Validation
    UFUNCTION(BlueprintCallable, Category = "Architecture", CallInEditor = true)
    bool ValidateProjectArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Architecture", CallInEditor = true)
    bool ValidateModuleDependencies();

    UFUNCTION(BlueprintCallable, Category = "Architecture", CallInEditor = true)
    bool ValidateHeaderIncludes();

    UFUNCTION(BlueprintCallable, Category = "Architecture", CallInEditor = true)
    bool ValidateUE5Compatibility();

    // Module Management
    UFUNCTION(BlueprintCallable, Category = "Module", CallInEditor = true)
    void RegisterModule(const FEng_ModuleInfo& ModuleInfo);

    UFUNCTION(BlueprintCallable, Category = "Module", CallInEditor = true)
    bool IsModuleValid(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Module", CallInEditor = true)
    TArray<FString> GetInvalidModules();

    // Validation Rules
    UFUNCTION(BlueprintCallable, Category = "Validation", CallInEditor = true)
    void AddValidationRule(const FEng_ValidationRule& Rule);

    UFUNCTION(BlueprintCallable, Category = "Validation", CallInEditor = true)
    TArray<FEng_ValidationRule> GetActiveValidationRules();

    // Compilation Checks
    UFUNCTION(BlueprintCallable, Category = "Compilation", CallInEditor = true)
    bool CheckCompilationErrors();

    UFUNCTION(BlueprintCallable, Category = "Compilation", CallInEditor = true)
    TArray<FString> GetCompilationErrors();

    // Performance Validation
    UFUNCTION(BlueprintCallable, Category = "Performance", CallInEditor = true)
    bool ValidatePerformanceConstraints();

    UFUNCTION(BlueprintCallable, Category = "Performance", CallInEditor = true)
    float GetCurrentFrameRate();

    UFUNCTION(BlueprintCallable, Category = "Performance", CallInEditor = true)
    int32 GetActorCount();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<FEng_ModuleInfo> RegisteredModules;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<FEng_ValidationRule> ValidationRules;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<FString> CompilationErrors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bArchitectureValid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActorCount;

private:
    void InitializeValidationRules();
    void InitializeModuleRegistry();
    bool ValidateModuleStructure(const FEng_ModuleInfo& Module);
    bool CheckHeaderCompliance(const FString& HeaderPath);
    void LogValidationResult(const FString& TestName, bool bPassed);
};

#include "Eng_ArchitectureValidator.generated.h"