#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Build_IntegrationValidator.h"
#include "Build_SystemOrchestrator.generated.h"

UENUM(BlueprintType)
enum class EBuild_SystemPriority : uint8
{
    Critical    UMETA(DisplayName = "Critical"),
    High        UMETA(DisplayName = "High"),
    Medium      UMETA(DisplayName = "Medium"),
    Low         UMETA(DisplayName = "Low")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "System")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    EBuild_SystemPriority Priority = EBuild_SystemPriority::Medium;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    bool bIsActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    bool bIsValidated = false;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    float LastValidationTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    TArray<FString> Dependencies;

    FBuild_SystemInfo()
    {
        SystemName = TEXT("Unknown");
        Priority = EBuild_SystemPriority::Medium;
        bIsActive = false;
        bIsValidated = false;
        LastValidationTime = 0.0f;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UBuild_SystemOrchestrator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void RegisterSystem(const FString& SystemName, EBuild_SystemPriority Priority, const TArray<FString>& Dependencies);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_ValidationResult GetOverallIntegrationStatus();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FBuild_SystemInfo> GetSystemsInfo() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool IsSystemActive(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void SetSystemActive(const FString& SystemName, bool bActive);

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    void InitializeDefaultSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    void RunFullIntegrationTest();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TMap<FString, FBuild_SystemInfo> RegisteredSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    UBuild_IntegrationValidator* IntegrationValidator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    float ValidationInterval = 30.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float LastFullValidationTime = 0.0f;

private:
    void RegisterCoreGameplaySystems();
    void RegisterWorldGenerationSystems();
    void RegisterAISystems();
    void RegisterQASystems();
    bool ValidateSystemDependencies(const FString& SystemName);
    void UpdateSystemValidationTime(const FString& SystemName);
};