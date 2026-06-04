#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Eng_GameplayArchitect.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_GameplayRequirement
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    FString RequirementName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    EGameplaySystemType SystemType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    bool bIsImplemented;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    bool bIsTested;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    float ImplementationProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    FString DependentSystems;

    FEng_GameplayRequirement()
    {
        RequirementName = TEXT("");
        SystemType = EGameplaySystemType::Movement;
        bIsImplemented = false;
        bIsTested = false;
        ImplementationProgress = 0.0f;
        DependentSystems = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_CharacterArchitecture
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    bool bHasMovementComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    bool bHasCameraComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    bool bHasInputBinding;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    bool bHasSurvivalStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float JumpHeight;

    FEng_CharacterArchitecture()
    {
        bHasMovementComponent = false;
        bHasCameraComponent = false;
        bHasInputBinding = false;
        bHasSurvivalStats = false;
        MovementSpeed = 600.0f;
        JumpHeight = 420.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_WorldArchitecture
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    bool bHasLandscape;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    bool bHasLighting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    bool bHasDinosaurs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    bool bHasNavMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    int32 DinosaurCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    float WorldSize;

    FEng_WorldArchitecture()
    {
        bHasLandscape = false;
        bHasLighting = false;
        bHasDinosaurs = false;
        bHasNavMesh = false;
        DinosaurCount = 0;
        WorldSize = 4000.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_GameplayArchitect : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_GameplayArchitect();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Gameplay architecture validation
    UFUNCTION(BlueprintCallable, Category = "Gameplay Architecture")
    bool ValidateWalkAroundMilestone();

    UFUNCTION(BlueprintCallable, Category = "Gameplay Architecture")
    bool ValidateCharacterImplementation();

    UFUNCTION(BlueprintCallable, Category = "Gameplay Architecture")
    bool ValidateWorldImplementation();

    UFUNCTION(BlueprintCallable, Category = "Gameplay Architecture")
    float GetMilestoneProgress();

    // Architecture requirements management
    UFUNCTION(BlueprintCallable, Category = "Gameplay Architecture")
    void AddGameplayRequirement(const FEng_GameplayRequirement& Requirement);

    UFUNCTION(BlueprintCallable, Category = "Gameplay Architecture")
    void UpdateRequirementProgress(const FString& RequirementName, float Progress);

    UFUNCTION(BlueprintCallable, Category = "Gameplay Architecture")
    TArray<FEng_GameplayRequirement> GetPendingRequirements();

    // System integration validation
    UFUNCTION(BlueprintCallable, Category = "Gameplay Architecture")
    bool ValidateSystemIntegration(EGameplaySystemType SystemA, EGameplaySystemType SystemB);

    UFUNCTION(BlueprintCallable, Category = "Gameplay Architecture")
    void RegisterSystemDependency(EGameplaySystemType System, EGameplaySystemType Dependency);

    // Real-time monitoring
    UFUNCTION(BlueprintCallable, Category = "Gameplay Architecture")
    void StartArchitectureMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Gameplay Architecture")
    void StopArchitectureMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Gameplay Architecture")
    FString GetArchitectureReport();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<FEng_GameplayRequirement> GameplayRequirements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FEng_CharacterArchitecture CharacterArch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FEng_WorldArchitecture WorldArch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bMonitoringActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float LastValidationTime;

private:
    void InitializeDefaultRequirements();
    void ValidateCurrentImplementation();
    bool CheckCharacterMovement();
    bool CheckWorldState();
    bool CheckDinosaurPresence();
    void LogArchitectureStatus(const FString& Message);

    FTimerHandle MonitoringTimerHandle;
};