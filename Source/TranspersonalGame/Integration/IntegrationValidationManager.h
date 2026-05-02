#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "IntegrationValidationManager.generated.h"

UENUM(BlueprintType)
enum class EInteg_ValidationResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Warning     UMETA(DisplayName = "Warning"),
    Fail        UMETA(DisplayName = "Fail"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FInteg_ValidationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    EInteg_ValidationResult Result;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString Message;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float ExecutionTime;

    FInteg_ValidationReport()
    {
        TestName = TEXT("");
        Result = EInteg_ValidationResult::Pass;
        Message = TEXT("");
        ExecutionTime = 0.0f;
    }

    FInteg_ValidationReport(const FString& InTestName, EInteg_ValidationResult InResult, const FString& InMessage, float InExecutionTime = 0.0f)
        : TestName(InTestName), Result(InResult), Message(InMessage), ExecutionTime(InExecutionTime)
    {
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FInteg_ActorInventory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Inventory")
    FString ActorClass;

    UPROPERTY(BlueprintReadOnly, Category = "Inventory")
    int32 Count;

    UPROPERTY(BlueprintReadOnly, Category = "Inventory")
    TArray<FString> ActorNames;

    FInteg_ActorInventory()
    {
        ActorClass = TEXT("");
        Count = 0;
        ActorNames.Empty();
    }
};

/**
 * Integration Validation Manager
 * Validates cross-system integration and maintains build quality
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UIntegrationValidationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UIntegrationValidationManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core validation methods
    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    TArray<FInteg_ValidationReport> RunFullValidationSuite();

    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    FInteg_ValidationReport ValidateLightingSetup();

    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    FInteg_ValidationReport ValidateCharacterSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    FInteg_ValidationReport ValidateDinosaurActors();

    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    FInteg_ValidationReport ValidateTerrainAndNavigation();

    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    FInteg_ValidationReport ValidateGameModeSetup();

    // Actor inventory and cleanup
    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    TArray<FInteg_ActorInventory> GenerateActorInventory();

    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    int32 CleanupDuplicateLightingActors();

    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    bool ValidateClassLoading(const FString& ClassName);

    // Performance validation
    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    FInteg_ValidationReport ValidatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    FInteg_ValidationReport ValidateMemoryUsage();

    // Build integration
    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    bool ValidateModuleIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    TArray<FString> GetMissingImplementations();

    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    bool GenerateIntegrationReport(const FString& OutputPath);

    // Editor-only validation
    UFUNCTION(CallInEditor, Category = "Integration Validation")
    void RunEditorValidation();

    UFUNCTION(CallInEditor, Category = "Integration Validation")
    void CleanupMapDuplicates();

protected:
    // Internal validation helpers
    bool ValidateActorCount(const FString& ActorClass, int32 ExpectedMin, int32 ExpectedMax, FString& OutMessage);
    bool ValidateComponentSetup(AActor* Actor, const TArray<FString>& RequiredComponents, FString& OutMessage);
    float MeasureValidationTime(TFunction<bool()> ValidationFunction);

    // Validation state
    UPROPERTY(BlueprintReadOnly, Category = "Integration Validation")
    TArray<FInteg_ValidationReport> LastValidationResults;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Validation")
    float LastValidationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Validation")
    bool bValidationInProgress;

private:
    // Critical thresholds
    static constexpr int32 MAX_DIRECTIONAL_LIGHTS = 1;
    static constexpr int32 MAX_SKY_ATMOSPHERES = 1;
    static constexpr int32 MAX_SKY_LIGHTS = 1;
    static constexpr int32 MAX_HEIGHT_FOGS = 1;
    static constexpr float MAX_VALIDATION_TIME = 30.0f;
};