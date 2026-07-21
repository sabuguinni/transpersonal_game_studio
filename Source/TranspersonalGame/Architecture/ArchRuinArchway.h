#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArchRuinArchway.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArchRuinArchway : public AActor
{
	GENERATED_BODY()

public:
	AArchRuinArchway();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture")
	UStaticMeshComponent* ArchMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture")
	UStaticMeshComponent* MossMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
	float WeatheringLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
	bool bHasVines;

	UFUNCTION(BlueprintCallable, Category = "Architecture")
	void ApplyWeathering(float Level);

protected:
	virtual void BeginPlay() override;
};
