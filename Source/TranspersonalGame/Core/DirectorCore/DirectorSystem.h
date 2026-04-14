#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "../../SharedTypes.h"
#include "DirectorSystem.generated.h"

/**
 * Studio Director System - Central coordination hub for all game systems
 * Manages inter-system communication and ensures proper initialization order
 */

UENUM(BlueprintType)
enum class EDir_SystemState : uint8
{
    Uninitialized,
    Initializing,
    Ready,
    Error,
    Shutdown
};

USTRUCT(BlueprintType)
struct FDir_SystemInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString SystemName;

    UPROPERTY(BlueprintReadOnly)
    EDir_SystemState State;

    UPROPERTY(BlueprintReadOnly)
    float InitializationTime;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> Dependencies;

    FDir_SystemInfo()
    {
        SystemName = TEXT("");
        State = EDir_SystemState::Uninitialized;
        InitializationTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FDir_SystemMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    int32 TotalSystems;

    UPROPERTY(BlueprintReadOnly)
    int32 ReadySystems;

    UPROPERTY(BlueprintReadOnly)
    int32 ErrorSystems;

    UPROPERTY(BlueprintReadOnly)
    float TotalInitTime;

    FDir_SystemMetrics()
    {
        TotalSystems = 0;
        ReadySystems = 0;
        ErrorSystems = 0;
        TotalInitTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDir_DirectorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDir_DirectorComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(BlueprintReadOnly, Category = "Director System")
    TMap<FString, FDir_SystemInfo> RegisteredSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Director System")
    FDir_SystemMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Director System")
    bool bAutoInitializeSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Director System")
    float SystemTimeoutSeconds;

public:
    UFUNCTION(BlueprintCallable, Category = "Director System")
    bool RegisterSystem(const FString& SystemName, const TArray<FString>& Dependencies);

    UFUNCTION(BlueprintCallable, Category = "Director System")
    void InitializeSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Director System")
    void InitializeAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Director System")
    EDir_SystemState GetSystemState(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Director System")
    FDir_SystemMetrics GetSystemMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Director System")
    TArray<FString> GetSystemsInState(EDir_SystemState State) const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Director System")
    void DebugPrintSystemStatus();

private:
    void UpdateMetrics();
    bool AreDependenciesMet(const FString& SystemName) const;
    void SetSystemState(const FString& SystemName, EDir_SystemState NewState);
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADir_DirectorActor : public AActor
{
    GENERATED_BODY()

public:
    ADir_DirectorActor();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UDir_DirectorComponent* DirectorComponent;

    virtual void BeginPlay() override;

public:
    UFUNCTION(BlueprintCallable, Category = "Director System")
    UDir_DirectorComponent* GetDirectorComponent() const { return DirectorComponent; }
};