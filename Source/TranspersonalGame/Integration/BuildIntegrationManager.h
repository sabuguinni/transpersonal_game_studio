#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "BuildIntegrationManager.generated.h"

UENUM(BlueprintType)
enum class EBuild_IntegrationStatus : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    Initializing    UMETA(DisplayName = "Initializing"),
    Validating      UMETA(DisplayName = "Validating"),
    Integrating     UMETA(DisplayName = "Integrating"),
    Complete        UMETA(DisplayName = "Complete"),
    Failed          UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    bool bIsLoaded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    bool bIsValidated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    float ValidationTime;

    FBuild_SystemStatus()
    {
        SystemName = TEXT("");
        bIsLoaded = false;
        bIsValidated = false;
        ErrorMessage = TEXT("");
        ValidationTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    TArray<FBuild_SystemStatus> SystemStatuses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    int32 TotalSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    int32 LoadedSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    int32 ValidatedSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    float IntegrationPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    EBuild_IntegrationStatus OverallStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    FDateTime LastValidationTime;

    FBuild_IntegrationReport()
    {
        TotalSystems = 0;
        LoadedSystems = 0;
        ValidatedSystems = 0;
        IntegrationPercentage = 0.0f;
        OverallStatus = EBuild_IntegrationStatus::Unknown;
        LastValidationTime = FDateTime::Now();
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABuildIntegrationManager : public AActor
{
    GENERATED_BODY()

public:
    ABuildIntegrationManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // === CORE INTEGRATION FUNCTIONS ===
    
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void StartIntegrationValidation();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateSystemIntegration(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void GenerateIntegrationReport();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_IntegrationReport GetCurrentIntegrationReport() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool IsSystemLoaded(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ResetIntegrationState();

    // === QA INTEGRATION ===
    
    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    void ValidateQAFramework();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    void RunQAIntegrationTests();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    bool IsQAFrameworkReady() const;

    // === VFX INTEGRATION ===
    
    UFUNCTION(BlueprintCallable, Category = "VFX Integration")
    void ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "VFX Integration")
    void TestVFXIntegration();

    UFUNCTION(BlueprintCallable, Category = "VFX Integration")
    bool AreVFXSystemsReady() const;

    // === AUDIO INTEGRATION ===
    
    UFUNCTION(BlueprintCallable, Category = "Audio Integration")
    void ValidateAudioSystems();

    UFUNCTION(BlueprintCallable, Category = "Audio Integration")
    void TestAudioIntegration();

    UFUNCTION(BlueprintCallable, Category = "Audio Integration")
    bool AreAudioSystemsReady() const;

    // === BUILD VALIDATION ===
    
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void ValidateBuildIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void CheckModuleDependencies();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool IsBuildStable() const;

protected:
    // === CORE PROPERTIES ===
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* IntegrationVisualizerMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    FBuild_IntegrationReport CurrentReport;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    TArray<FString> CoreSystemNames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    bool bAutoValidateOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    float ValidationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    bool bContinuousValidation;

    // === QA PROPERTIES ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Integration")
    bool bQAFrameworkValidated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Integration")
    TArray<FString> QATestResults;

    // === VFX PROPERTIES ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Integration")
    bool bVFXSystemsValidated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Integration")
    TArray<FString> VFXValidationResults;

    // === AUDIO PROPERTIES ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Integration")
    bool bAudioSystemsValidated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Integration")
    TArray<FString> AudioValidationResults;

private:
    // === INTERNAL VALIDATION ===
    
    void InitializeCoreSystemNames();
    void UpdateIntegrationStatus();
    void LogIntegrationResults();
    
    // Validation timers
    float LastValidationTime;
    bool bValidationInProgress;
};