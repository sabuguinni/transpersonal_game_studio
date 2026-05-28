#pragma once
#include "SharedTypes.h"

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "GeneratedTypeStubs.h"
#include "CS_MigrationActorManager.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACS_CrowdMigration : public AActor
{
    GENERATED_BODY()

public:
    ACS_CrowdMigration();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    TArray<FCrowd_MigrationRoute> MigrationRoutes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    float SeasonalMigrationInterval = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    int32 MaxSimultaneousMigrations = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    float GroupFormationRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    bool bEnableSeasonalMigration = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    int32 ActiveMigrationCount = 0;

    UFUNCTION(BlueprintCallable, Category = "Migration")
    void StartSeasonalMigration();

    UFUNCTION(BlueprintCallable, Category = "Migration")
    void StopAllMigrations();

    UFUNCTION(BlueprintCallable, Category = "Migration")
    int32 GetActiveMigrationCount() const;

private:
    UPROPERTY()
    USceneComponent* RootSceneComponent;

    float LastSeasonalCheck = 0.0f;
};
