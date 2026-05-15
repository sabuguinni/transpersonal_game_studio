#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "Materials/MaterialInterface.h"
#include "Char_TribalCharacter.generated.h"

UENUM(BlueprintType)
enum class EChar_TribalGender : uint8
{
    Male UMETA(DisplayName = "Male"),
    Female UMETA(DisplayName = "Female"),
    Child UMETA(DisplayName = "Child")
};

UENUM(BlueprintType)
enum class EChar_TribalRole : uint8
{
    Hunter UMETA(DisplayName = "Hunter"),
    Gatherer UMETA(DisplayName = "Gatherer"),
    Shaman UMETA(DisplayName = "Shaman"),
    Crafter UMETA(DisplayName = "Crafter"),
    Scout UMETA(DisplayName = "Scout"),
    Elder UMETA(DisplayName = "Elder")
};

USTRUCT(BlueprintType)
struct FChar_TribalAppearance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float BodyMassIndex = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float Height = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasScars = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasTattoos = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    int32 Age = 25;

    FChar_TribalAppearance()
    {
        SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
        HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f);
        BodyMassIndex = 1.0f;
        Height = 1.0f;
        bHasScars = false;
        bHasTattoos = false;
        Age = 25;
    }
};

USTRUCT(BlueprintType)
struct FChar_TribalClothing
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<UStaticMesh> ChestClothing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<UStaticMesh> LegClothing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<UStaticMesh> FootClothing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<UStaticMesh> HeadAccessory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<UMaterialInterface> ClothingMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    FLinearColor ClothingColor = FLinearColor(0.6f, 0.4f, 0.2f, 1.0f);

    FChar_TribalClothing()
    {
        ClothingColor = FLinearColor(0.6f, 0.4f, 0.2f, 1.0f);
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API AChar_TribalCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AChar_TribalCharacter();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Character Appearance System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Character")
    EChar_TribalGender Gender = EChar_TribalGender::Male;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Character")
    EChar_TribalRole TribalRole = EChar_TribalRole::Hunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Character")
    FChar_TribalAppearance Appearance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Character")
    FChar_TribalClothing Clothing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Character")
    FString CharacterName = TEXT("Tribal Survivor");

    // Camera System
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    USpringArmComponent* SpringArm;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    UCameraComponent* FollowCamera;

    // Clothing Mesh Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Clothing")
    UStaticMeshComponent* ChestClothingMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Clothing")
    UStaticMeshComponent* LegClothingMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Clothing")
    UStaticMeshComponent* FootClothingMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Clothing")
    UStaticMeshComponent* HeadAccessoryMesh;

    // Character Customization Functions
    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    void SetCharacterGender(EChar_TribalGender NewGender);

    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    void SetTribalRole(EChar_TribalRole NewRole);

    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    void SetCharacterAppearance(const FChar_TribalAppearance& NewAppearance);

    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    void SetCharacterClothing(const FChar_TribalClothing& NewClothing);

    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    void ApplyRandomTribalAppearance();

    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    void SetupDefaultMannequinMesh();

    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    void ConfigureCameraSystem(float ArmLength = 400.0f, float ArmHeight = 50.0f);

    // Material and Mesh Management
    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    void UpdateSkinMaterial();

    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    void UpdateClothingMeshes();

    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    void SetSkeletalMeshFromPath(const FString& MeshPath);

protected:
    // Internal helper functions
    void InitializeCharacterComponents();
    void SetupDefaultAppearance();
    void ApplyAppearanceToMesh();
    FLinearColor GenerateRandomSkinTone();
    FLinearColor GenerateRandomHairColor();

    // Movement configuration
    void ConfigureMovementSettings();

private:
    // Character state tracking
    bool bIsInitialized = false;
    float LastAppearanceUpdateTime = 0.0f;
};