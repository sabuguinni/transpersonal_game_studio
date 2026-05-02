#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "QA_ValidationManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_ValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    bool bPassed;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float ExecutionTimeMs;

    FQA_ValidationResult()
    {
        TestName = TEXT("");
        bPassed = false;
        ErrorMessage = TEXT("");
        ExecutionTimeMs = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_ValidationSuite
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    TArray<FQA_ValidationResult> TestResults;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float OverallScore;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 TotalTests;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 PassedTests;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FDateTime ExecutionTime;

    FQA_ValidationSuite()
    {
        OverallScore = 0.0f;
        TotalTests = 0;
        PassedTests = 0;
        ExecutionTime = FDateTime::Now();
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_ValidationManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_ValidationManager();

protected:
    virtual void BeginPlay() override;

public:
    // Executar suite completa de validação
    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_ValidationSuite RunFullValidationSuite();

    // Testes individuais
    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_ValidationResult ValidateEssentialActors();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_ValidationResult ValidateClassLoading();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_ValidationResult ValidateLightingSetup();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_ValidationResult ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_ValidationResult ValidateCharacterSystems();

    // Utilitários
    UFUNCTION(BlueprintCallable, Category = "QA")
    void LogValidationResults(const FQA_ValidationSuite& ValidationSuite);

    UFUNCTION(BlueprintCallable, Category = "QA")
    bool SaveValidationReport(const FQA_ValidationSuite& ValidationSuite, const FString& FilePath);

protected:
    // Configuração de validação
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    bool bAutoRunOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    bool bLogDetailedResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    float ValidationTimeoutSeconds;

    // Classes críticas para validar
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    TArray<FString> CriticalClassPaths;

    // Tipos de actores essenciais
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    TMap<FString, int32> EssentialActorTypes;

private:
    // Helpers internos
    FQA_ValidationResult CreateTestResult(const FString& TestName, bool bPassed, const FString& ErrorMessage = TEXT(""), float ExecutionTime = 0.0f);
    
    TArray<AActor*> GetActorsOfType(const FString& ActorTypeName);
    
    bool IsClassLoadable(const FString& ClassPath);
    
    void InitializeDefaultSettings();
};