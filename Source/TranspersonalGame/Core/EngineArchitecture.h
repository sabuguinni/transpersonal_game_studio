#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EngineArchitecture.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEngineArchitecture : public UActorComponent
{
    GENERATED_BODY()

public:
    UEngineArchitecture();

    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable, Category="Architecture")
    void ValidateCoreArchitecture();

    UFUNCTION(BlueprintCallable, Category="Architecture")
    void ValidateModuleSystems();

    UFUNCTION(BlueprintCallable, Category="Architecture")
    void ValidatePerformanceConstraints();

    UFUNCTION(BlueprintCallable, Category="Architecture")
    bool ValidateSystemIntegration(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category="Architecture")
    void ReportArchitectureStatus();
};
