#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "BuildIntegrationManager.generated.h"

UENUM(BlueprintType)
enum class EBuild_SystemStatus : uint8
{
    Unknown     UMETA(DisplayName = "Unknown"),
    Loading     UMETA(DisplayName = "Loading"),
    Ready       UMETA(DisplayName = "Ready"),
    Error       UMETA(DisplayName = "Error"),
    Disabled    UMETA(DisplayName = "Disabled")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "System")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    EBuild_SystemStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    float LastUpdateTime;

    FBuild_SystemInfo()
    {
        SystemName = TEXT("");
        Status = EBuild_SystemStatus::Unknown;
        ErrorMessage = TEXT("");
        ActorCount = 0;
        LastUpdateTime = 0.0f;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UBuildIntegrationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuildIntegrationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool IsSystemReady(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FBuild_SystemInfo> GetSystemStatus() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void RegisterSystem(const FString& SystemName, UClass* SystemClass);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void UpdateSystemStatus(const FString& SystemName, EBuild_SystemStatus NewStatus, const FString& ErrorMsg = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "Integration")
    int32 GetTotalActorCount() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void SaveMapSafely();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool CompileAndValidateModule(const FString& ModuleName);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TMap<FString, FBuild_SystemInfo> SystemRegistry;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bAllSystemsReady;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float LastValidationTime;

    void ValidateCharacterSystem();
    void ValidateWorldGeneration();
    void ValidateVFXSystem();
    void ValidateDinosaurAI();
    void ValidateCrowdSimulation();
    void ValidateFoliageSystem();

    void LogSystemStatus() const;
};