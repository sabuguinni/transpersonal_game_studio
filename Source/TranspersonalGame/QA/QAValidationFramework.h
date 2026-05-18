#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "QAValidationFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_ValidationResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning"),
    NotTested   UMETA(DisplayName = "Not Tested")
};

USTRUCT(BlueprintType)
struct FQA_SystemValidationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA Validation")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "QA Validation")
    EQA_ValidationResult ValidationResult;

    UPROPERTY(BlueprintReadOnly, Category = "QA Validation")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "QA Validation")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "QA Validation")
    float PerformanceScore;

    FQA_SystemValidationReport()
    {
        SystemName = TEXT("");
        ValidationResult = EQA_ValidationResult::NotTested;
        ErrorMessage = TEXT("");
        ActorCount = 0;
        PerformanceScore = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FQA_BiomeValidationData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA Validation")
    FString BiomeName;

    UPROPERTY(BlueprintReadOnly, Category = "QA Validation")
    FVector BiomeCenter;

    UPROPERTY(BlueprintReadOnly, Category = "QA Validation")
    int32 ActorPopulation;

    UPROPERTY(BlueprintReadOnly, Category = "QA Validation")
    bool bMeetsRequirement;

    UPROPERTY(BlueprintReadOnly, Category = "QA Validation")
    TArray<FString> ActorTypes;

    FQA_BiomeValidationData()
    {
        BiomeName = TEXT("");
        BiomeCenter = FVector::ZeroVector;
        ActorPopulation = 0;
        bMeetsRequirement = false;
    }
};

/**
 * QA Validation Framework - Comprehensive testing system for TranspersonalGame
 * Validates all core systems, integration points, and performance metrics
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQAValidationFramework : public AActor
{
    GENERATED_BODY()

public:
    AQAValidationFramework();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core System Validation
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_SystemValidationReport ValidateCharacterSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_SystemValidationReport ValidateVFXSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_SystemValidationReport ValidateAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_SystemValidationReport ValidatePhysicsSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_SystemValidationReport ValidateDinosaurSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_SystemValidationReport ValidateWorldGenerationSystem();

    // Biome Population Validation
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    TArray<FQA_BiomeValidationData> ValidateBiomePopulation();

    // Integration Testing
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateVFXAudioIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateCharacterDinosaurInteraction();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidatePhysicsCollisionIntegration();

    // Performance Validation
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    float CalculateSystemHealthScore();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    TArray<FString> DetectCriticalIssues();

    // Automated Testing
    UFUNCTION(BlueprintCallable, Category = "QA Validation", CallInEditor = true)
    void RunFullValidationSuite();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void GenerateValidationReport();

protected:
    // Validation Data
    UPROPERTY(BlueprintReadOnly, Category = "QA Validation")
    TArray<FQA_SystemValidationReport> SystemReports;

    UPROPERTY(BlueprintReadOnly, Category = "QA Validation")
    TArray<FQA_BiomeValidationData> BiomeReports;

    UPROPERTY(BlueprintReadOnly, Category = "QA Validation")
    float OverallHealthScore;

    UPROPERTY(BlueprintReadOnly, Category = "QA Validation")
    TArray<FString> CriticalIssues;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Configuration")
    int32 MinimumActorsPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Configuration")
    float BiomeValidationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Configuration")
    bool bAutoRunValidationOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Configuration")
    float ValidationTickInterval;

private:
    // Helper Functions
    bool IsClassLoaded(const FString& ClassName);
    int32 CountActorsOfClass(UClass* ActorClass);
    int32 CountActorsInRadius(const FVector& Center, float Radius);
    TArray<AActor*> GetActorsInRadius(const FVector& Center, float Radius);
    
    // Validation Timer
    float LastValidationTime;
};