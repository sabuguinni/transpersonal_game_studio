#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "QA_ValidationFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_ValidationResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct FQA_TestResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    EQA_ValidationResult Result;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString Message;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float ExecutionTime;

    FQA_TestResult()
    {
        TestName = TEXT("");
        Result = EQA_ValidationResult::Pass;
        Message = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FQA_SystemHealth
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 TotalActors;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 StaticMeshActors;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 SkeletalMeshActors;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 VFXActors;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 AIActors;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float MemoryUsagePercent;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 CriticalSystemsLoaded;

    FQA_SystemHealth()
    {
        TotalActors = 0;
        StaticMeshActors = 0;
        SkeletalMeshActors = 0;
        VFXActors = 0;
        AIActors = 0;
        MemoryUsagePercent = 0.0f;
        CriticalSystemsLoaded = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQA_ValidationFramework : public AActor
{
    GENERATED_BODY()

public:
    AQA_ValidationFramework();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(BlueprintReadOnly, Category = "QA", meta = (AllowPrivateAccess = "true"))
    TArray<FQA_TestResult> TestResults;

    UPROPERTY(BlueprintReadOnly, Category = "QA", meta = (AllowPrivateAccess = "true"))
    FQA_SystemHealth CurrentSystemHealth;

    UPROPERTY(BlueprintReadOnly, Category = "QA", meta = (AllowPrivateAccess = "true"))
    bool bValidationInProgress;

    UPROPERTY(BlueprintReadOnly, Category = "QA", meta = (AllowPrivateAccess = "true"))
    float ValidationStartTime;

public:
    UFUNCTION(BlueprintCallable, Category = "QA")
    void RunFullValidation();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ValidateCharacterSystems();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ValidateDinosaurAssets();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ValidatePhysicsSystems();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ValidateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ValidateAISystems();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_SystemHealth GetSystemHealth() const;

    UFUNCTION(BlueprintCallable, Category = "QA")
    TArray<FQA_TestResult> GetTestResults() const;

    UFUNCTION(BlueprintCallable, Category = "QA")
    bool IsValidationPassing() const;

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ClearTestResults();

    UFUNCTION(CallInEditor, Category = "QA")
    void EditorRunValidation();

private:
    void AddTestResult(const FString& TestName, EQA_ValidationResult Result, const FString& Message);
    void UpdateSystemHealth();
    bool ValidateClassExists(const FString& ClassName);
    int32 CountActorsOfType(const FString& TypeName);
};