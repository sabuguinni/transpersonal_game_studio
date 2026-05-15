#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "QA_ValidationFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_ValidationResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning"),
    Critical    UMETA(DisplayName = "Critical")
};

UENUM(BlueprintType)
enum class EQA_SystemType : uint8
{
    Character   UMETA(DisplayName = "Character System"),
    VFX         UMETA(DisplayName = "VFX System"),
    WorldGen    UMETA(DisplayName = "World Generation"),
    GameState   UMETA(DisplayName = "Game State"),
    Physics     UMETA(DisplayName = "Physics System"),
    Audio       UMETA(DisplayName = "Audio System"),
    AI          UMETA(DisplayName = "AI System"),
    UI          UMETA(DisplayName = "UI System")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_ValidationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    EQA_SystemType SystemType;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    EQA_ValidationResult Result;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    FString TestName;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    FString Description;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    float ExecutionTime;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    FDateTime Timestamp;

    FQA_ValidationReport()
    {
        SystemType = EQA_SystemType::Character;
        Result = EQA_ValidationResult::Pass;
        TestName = TEXT("");
        Description = TEXT("");
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
        Timestamp = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_BiomePopulation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    FString BiomeName;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    FVector BiomeCenter;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    int32 ActorCount;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    int32 RequiredMinimum;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    bool bMeetsRequirement;

    FQA_BiomePopulation()
    {
        BiomeName = TEXT("");
        BiomeCenter = FVector::ZeroVector;
        ActorCount = 0;
        RequiredMinimum = 500;
        bMeetsRequirement = false;
    }
};

/**
 * QA Validation Framework Actor
 * Provides comprehensive testing and validation for all game systems
 * Monitors performance, memory usage, and system integration
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQA_ValidationFramework : public AActor
{
    GENERATED_BODY()

public:
    AQA_ValidationFramework();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "QA")
    void RunFullValidationSuite();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_ValidationReport ValidateCharacterSystem();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_ValidationReport ValidateVFXSystem();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_ValidationReport ValidateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_ValidationReport ValidateGameState();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_ValidationReport ValidatePhysicsSystem();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_ValidationReport ValidateAudioSystem();

    // Biome and population validation
    UFUNCTION(BlueprintCallable, Category = "QA")
    TArray<FQA_BiomePopulation> ValidateBiomePopulation();

    UFUNCTION(BlueprintCallable, Category = "QA")
    int32 GetActorCountInRadius(FVector Center, float Radius);

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "QA")
    float GetCurrentFPS();

    UFUNCTION(BlueprintCallable, Category = "QA")
    float GetMemoryUsagePercent();

    UFUNCTION(BlueprintCallable, Category = "QA")
    int32 GetTotalActorCount();

    // Asset validation
    UFUNCTION(BlueprintCallable, Category = "QA")
    bool ValidateAssetImportPipeline();

    UFUNCTION(BlueprintCallable, Category = "QA")
    bool TestFBXImportCapability();

    // Integration testing
    UFUNCTION(BlueprintCallable, Category = "QA")
    bool TestCrossSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA")
    bool ValidateRemoteControlAPI();

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "QA")
    void GenerateValidationReport();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void LogValidationResults(const TArray<FQA_ValidationReport>& Reports);

    UFUNCTION(BlueprintCallable, Category = "QA")
    float CalculateSystemHealthPercentage();

protected:
    // Validation data
    UPROPERTY(BlueprintReadOnly, Category = "QA")
    TArray<FQA_ValidationReport> ValidationReports;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    TArray<FQA_BiomePopulation> BiomePopulations;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    bool bAutoRunValidation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    float ValidationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    bool bLogDetailedResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    bool bGenerateReports;

    // Performance thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    float MinimumFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    float MaxMemoryUsagePercent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    int32 MaxActorCount;

    // Biome centers (from brain memory coordinates)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FVector SavanaCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FVector PantanoCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FVector FlorestaCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FVector DesertoCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FVector MontanhaCenter;

private:
    // Internal validation helpers
    bool ValidateClassLoading(const FString& ClassName);
    bool ValidateActorSpawning(UClass* ActorClass);
    bool ValidateComponentFunctionality(AActor* TestActor);
    
    // Timer for automatic validation
    FTimerHandle ValidationTimerHandle;
    
    // Performance tracking
    float LastValidationTime;
    int32 ValidationRunCount;
};