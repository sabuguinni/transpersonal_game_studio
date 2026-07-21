#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Engine.h"
#include "SystemIntegrationValidator.generated.h"

UENUM(BlueprintType)
enum class EBuild_ValidationResult : uint8
{
    NotTested       UMETA(DisplayName = "Not Tested"),
    Passed          UMETA(DisplayName = "Passed"),
    Failed          UMETA(DisplayName = "Failed"),
    Warning         UMETA(DisplayName = "Warning"),
    Critical        UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ValidationTest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    FString TestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    EBuild_ValidationResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    FString ResultMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    float ExecutionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    FDateTime TestTime;

    FBuild_ValidationTest()
    {
        TestName = TEXT("");
        TestDescription = TEXT("");
        Result = EBuild_ValidationResult::NotTested;
        ResultMessage = TEXT("");
        ExecutionTime = 0.0f;
        TestTime = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ModuleValidation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module Validation")
    FString ModuleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module Validation")
    bool bIsLoaded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module Validation")
    bool bHasValidClasses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module Validation")
    TArray<FString> LoadedClasses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module Validation")
    TArray<FString> FailedClasses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module Validation")
    TArray<FBuild_ValidationTest> ValidationTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module Validation")
    EBuild_ValidationResult OverallResult;

    FBuild_ModuleValidation()
    {
        ModuleName = TEXT("");
        bIsLoaded = false;
        bHasValidClasses = false;
        OverallResult = EBuild_ValidationResult::NotTested;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API USystemIntegrationValidator : public UObject
{
    GENERATED_BODY()

public:
    USystemIntegrationValidator();

    // === CORE VALIDATION FUNCTIONS ===
    
    UFUNCTION(BlueprintCallable, Category = "System Validation")
    void RunFullSystemValidation();

    UFUNCTION(BlueprintCallable, Category = "System Validation")
    void ValidateModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "System Validation")
    void ValidateClassLoading(const FString& ClassName);

    UFUNCTION(BlueprintCallable, Category = "System Validation")
    void ValidateActorSpawning(const FString& ActorClassName);

    UFUNCTION(BlueprintCallable, Category = "System Validation")
    FBuild_ValidationTest CreateValidationTest(const FString& TestName, const FString& Description);

    UFUNCTION(BlueprintCallable, Category = "System Validation")
    void ExecuteValidationTest(UPARAM(ref) FBuild_ValidationTest& Test);

    // === SPECIALIZED VALIDATIONS ===
    
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void ValidateQAIntegration();

    UFUNCTION(BlueprintCallable, Category = "VFX Validation")
    void ValidateVFXIntegration();

    UFUNCTION(BlueprintCallable, Category = "Audio Validation")
    void ValidateAudioIntegration();

    UFUNCTION(BlueprintCallable, Category = "World Validation")
    void ValidateWorldSystems();

    UFUNCTION(BlueprintCallable, Category = "Character Validation")
    void ValidateCharacterSystems();

    // === REPORTING FUNCTIONS ===
    
    UFUNCTION(BlueprintCallable, Category = "Validation Reporting")
    void GenerateValidationReport();

    UFUNCTION(BlueprintCallable, Category = "Validation Reporting")
    TArray<FBuild_ModuleValidation> GetModuleValidations() const;

    UFUNCTION(BlueprintCallable, Category = "Validation Reporting")
    TArray<FBuild_ValidationTest> GetAllValidationTests() const;

    UFUNCTION(BlueprintCallable, Category = "Validation Reporting")
    int32 GetPassedTestCount() const;

    UFUNCTION(BlueprintCallable, Category = "Validation Reporting")
    int32 GetFailedTestCount() const;

    UFUNCTION(BlueprintCallable, Category = "Validation Reporting")
    float GetValidationSuccessRate() const;

    // === UTILITY FUNCTIONS ===
    
    UFUNCTION(BlueprintCallable, Category = "Validation Utilities")
    void ClearValidationResults();

    UFUNCTION(BlueprintCallable, Category = "Validation Utilities")
    void LogValidationResults();

    UFUNCTION(BlueprintCallable, Category = "Validation Utilities")
    bool IsValidationComplete() const;

    UFUNCTION(BlueprintCallable, Category = "Validation Utilities")
    void SaveValidationReport(const FString& FilePath);

protected:
    // === VALIDATION DATA ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Data")
    TArray<FBuild_ModuleValidation> ModuleValidations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Data")
    TArray<FBuild_ValidationTest> AllValidationTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Data")
    TArray<FString> CoreModuleNames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Data")
    TArray<FString> CoreClassNames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Data")
    bool bValidationInProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Data")
    FDateTime LastValidationTime;

    // === QA VALIDATION DATA ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Validation")
    TArray<FString> QAClassNames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Validation")
    bool bQAValidationComplete;

    // === VFX VALIDATION DATA ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Validation")
    TArray<FString> VFXClassNames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Validation")
    bool bVFXValidationComplete;

    // === AUDIO VALIDATION DATA ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Validation")
    TArray<FString> AudioClassNames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Validation")
    bool bAudioValidationComplete;

private:
    // === INTERNAL VALIDATION HELPERS ===
    
    void InitializeCoreModules();
    void InitializeCoreClasses();
    void InitializeSpecializedClasses();
    
    FBuild_ValidationTest ValidateClassExistence(const FString& ClassName);
    FBuild_ValidationTest ValidateActorSpawn(const FString& ActorClassName);
    FBuild_ValidationTest ValidateModuleLoad(const FString& ModuleName);
    
    void UpdateModuleValidation(const FString& ModuleName, const FBuild_ValidationTest& Test);
    void LogTestResult(const FBuild_ValidationTest& Test);
};