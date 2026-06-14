#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "Eng_WorldArchitect.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_WorldArchitect : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void InitializeWorldSystems();

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void ValidateWorldConfiguration();

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    bool IsWorldSystemsReady() const;

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void RegisterArchitecturalComponent(UActorComponent* Component, EArchitecturalLayer Layer);

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    TArray<UActorComponent*> GetComponentsByLayer(EArchitecturalLayer Layer) const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    bool bWorldSystemsInitialized;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TMap<EArchitecturalLayer, TArray<TWeakObjectPtr<UActorComponent>>> LayeredComponents;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    float WorldValidationScore;

private:
    void ValidatePerformanceConstraints();
    void ValidateMemoryUsage();
    void ValidateRenderingPipeline();
};