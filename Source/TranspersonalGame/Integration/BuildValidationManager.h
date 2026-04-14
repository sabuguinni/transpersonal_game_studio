#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "BuildValidationManager.generated.h"

UENUM(BlueprintType)
enum class EBuild_ValidationResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    PartialPass UMETA(DisplayName = "Partial Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    NotRun      UMETA(DisplayName = "Not Run")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ValidationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    EBuild_ValidationResult Result = EBuild_ValidationResult::NotRun;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 ClassesLoaded = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 ClassesExpected = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 BinaryFilesFound = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 SourceFilesFound = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> FailedClasses;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString ValidationTimestamp;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float ValidationDuration = 0.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABuildValidationManager : public AActor
{
    GENERATED_BODY()

public:
    ABuildValidationManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    FBuild_ValidationReport RunFullValidation();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool ValidateClassLoading();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool ValidateBinaryFiles();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool ValidateSourceFiles();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void RunIntegrationTests();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void GenerateValidationReport();

    // Getters
    UFUNCTION(BlueprintPure, Category = "Build Validation")
    FBuild_ValidationReport GetLastValidationReport() const { return LastValidationReport; }

    UFUNCTION(BlueprintPure, Category = "Build Validation")
    bool IsValidationPassing() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FBuild_ValidationReport LastValidationReport;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bAutoRunValidationOnBeginPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float ValidationInterval = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    TArray<FString> ExpectedClasses;

private:
    float TimeSinceLastValidation = 0.0f;
    bool bValidationInProgress = false;

    // Helper functions
    void InitializeExpectedClasses();
    bool CheckClassExists(const FString& ClassName);
    int32 CountBinaryFiles();
    int32 CountSourceFiles();
    void LogValidationResults(const FBuild_ValidationReport& Report);
};