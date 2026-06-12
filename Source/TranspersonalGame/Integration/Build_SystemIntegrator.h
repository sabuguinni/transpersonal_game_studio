#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Build_SystemIntegrator.generated.h"

UENUM(BlueprintType)
enum class EBuild_SystemStatus : uint8
{
    Uninitialized   UMETA(DisplayName = "Uninitialized"),
    Initializing    UMETA(DisplayName = "Initializing"),
    Ready           UMETA(DisplayName = "Ready"),
    Error           UMETA(DisplayName = "Error")
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
    FString Version;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    float InitializationTime;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    TArray<FString> Dependencies;

    FBuild_SystemInfo()
    {
        SystemName = TEXT("");
        Status = EBuild_SystemStatus::Uninitialized;
        Version = TEXT("1.0.0");
        InitializationTime = 0.0f;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UBuild_SystemIntegrator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_SystemIntegrator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // System registration and management
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool RegisterSystem(const FString& SystemName, const FString& Version, const TArray<FString>& Dependencies);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool UnregisterSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    EBuild_SystemStatus GetSystemStatus(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FBuild_SystemInfo> GetAllSystemInfo() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool InitializeAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ShutdownAllSystems();

    // Cross-system communication
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool SendSystemMessage(const FString& FromSystem, const FString& ToSystem, const FString& Message);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FString> GetSystemMessages(const FString& SystemName);

    // Integration validation
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FString GetIntegrationReport() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Systems")
    TMap<FString, FBuild_SystemInfo> RegisteredSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bIntegrationValid;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString LastIntegrationReport;

private:
    bool InitializeSystem(const FString& SystemName);
    bool CheckSystemDependencies(const FString& SystemName) const;
    void UpdateSystemStatus(const FString& SystemName, EBuild_SystemStatus NewStatus);
    
    // Message system
    TMap<FString, TArray<FString>> SystemMessages;
    
    // Integration timing
    float TotalInitializationTime;
};