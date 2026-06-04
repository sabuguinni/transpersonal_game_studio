#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "Eng_MilestoneValidator.generated.h"

UENUM(BlueprintType)
enum class EEng_MilestoneStatus : uint8
{
    NotStarted      UMETA(DisplayName = "Not Started"),
    InProgress      UMETA(DisplayName = "In Progress"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed"),
    Blocked         UMETA(DisplayName = "Blocked")
};

UENUM(BlueprintType)
enum class EEng_PrototypeComponent : uint8
{
    ThirdPersonCharacter    UMETA(DisplayName = "Third Person Character"),
    CameraBoom             UMETA(DisplayName = "Camera Boom"),
    BasicTerrain           UMETA(DisplayName = "Basic Terrain"),
    WASDMovement           UMETA(DisplayName = "WASD Movement"),
    DinosaurMeshes         UMETA(DisplayName = "Dinosaur Meshes"),
    DirectionalLight       UMETA(DisplayName = "Directional Light"),
    SkyAtmosphere          UMETA(DisplayName = "Sky Atmosphere"),
    FogSystem              UMETA(DisplayName = "Fog System")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_MilestoneRequirement
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString RequirementName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    EEng_MilestoneStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString AssignedAgent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FDateTime LastUpdated;

    FEng_MilestoneRequirement()
    {
        RequirementName = TEXT("");
        Description = TEXT("");
        Status = EEng_MilestoneStatus::NotStarted;
        CompletionPercentage = 0.0f;
        AssignedAgent = TEXT("");
        LastUpdated = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PrototypeValidation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    EEng_PrototypeComponent Component;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    bool bIsPresent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    bool bIsFunctional;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    FString ValidationNotes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    FDateTime LastChecked;

    FEng_PrototypeValidation()
    {
        Component = EEng_PrototypeComponent::ThirdPersonCharacter;
        bIsPresent = false;
        bIsFunctional = false;
        ValidationNotes = TEXT("");
        LastChecked = FDateTime::Now();
    }
};

/**
 * Engine Architect Milestone Validator
 * Tracks progress on MILESTONE 1: "WALK AROUND" prototype
 * Validates all required components for minimum viable playable prototype
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_MilestoneValidator : public UActorComponent
{
    GENERATED_BODY()

public:
    UEng_MilestoneValidator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === MILESTONE TRACKING ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone 1")
    TArray<FEng_MilestoneRequirement> Milestone1Requirements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone 1")
    TArray<FEng_PrototypeValidation> PrototypeComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone 1")
    float OverallCompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone 1")
    bool bMilestone1Complete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    float ValidationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    bool bAutoValidate;

    // === VALIDATION FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Validation")
    void ValidateAllComponents();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateCharacterSystem();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateTerrainSystem();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateLightingSystem();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateDinosaurSystem();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateMovementSystem();

    // === MILESTONE MANAGEMENT ===
    UFUNCTION(BlueprintCallable, Category = "Milestone")
    void UpdateRequirementStatus(const FString& RequirementName, EEng_MilestoneStatus NewStatus, float Percentage);

    UFUNCTION(BlueprintCallable, Category = "Milestone")
    float CalculateOverallProgress();

    UFUNCTION(BlueprintCallable, Category = "Milestone")
    TArray<FString> GetBlockingIssues();

    UFUNCTION(BlueprintCallable, Category = "Milestone")
    void GenerateProgressReport();

    // === ARCHITECTURE VALIDATION ===
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateEngineArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool CheckCompilationStatus();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FString> GetArchitecturalIssues();

private:
    float LastValidationTime;
    
    void InitializeMilestone1Requirements();
    void InitializePrototypeComponents();
    void LogValidationResults();
};