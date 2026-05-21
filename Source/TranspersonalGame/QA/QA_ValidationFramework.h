#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "QA_ValidationFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_ValidationResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning"),
    Error       UMETA(DisplayName = "Error")
};

USTRUCT(BlueprintType)
struct FQA_ValidationTest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FString TestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    EQA_ValidationResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    float ExecutionTime;

    FQA_ValidationTest()
    {
        TestName = TEXT("");
        TestDescription = TEXT("");
        Result = EQA_ValidationResult::Pass;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FQA_BiomeValidation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FVector BiomeCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    int32 ActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    int32 DinosaurCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    int32 StaticMeshCount;

    FQA_BiomeValidation()
    {
        BiomeName = TEXT("");
        BiomeCenter = FVector::ZeroVector;
        ActorCount = 0;
        DinosaurCount = 0;
        StaticMeshCount = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_ValidationFramework : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_ValidationFramework();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Validation Tests
    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_ValidationTest ValidateBridgeConnectivity();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_ValidationTest ValidateVFXSystem();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_ValidationTest ValidateCharacterSystem();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_ValidationTest ValidatePhysicsSystem();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    TArray<FQA_BiomeValidation> ValidateBiomePopulation();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_ValidationTest ValidateAssetPipeline();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_ValidationTest ValidatePerformanceMetrics();

    // Comprehensive validation
    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    TArray<FQA_ValidationTest> RunAllValidationTests();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "QA")
    float GetCurrentMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "QA")
    int32 GetTotalActorCount();

    UFUNCTION(BlueprintCallable, Category = "QA")
    bool IsPerformanceWithinLimits();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    TArray<FQA_ValidationTest> ValidationResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    TArray<FQA_BiomeValidation> BiomeValidationResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    float MaxMemoryUsagePercent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    int32 MaxActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    bool bAutoRunValidation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    float ValidationInterval;

private:
    float LastValidationTime;
    
    // Helper functions
    bool IsClassLoadable(const FString& ClassPath);
    bool CanSpawnActor(UClass* ActorClass);
    float CalculateDistance2D(const FVector& Pos1, const FVector& Pos2);
    FString GetBiomeForLocation(const FVector& Location);
};