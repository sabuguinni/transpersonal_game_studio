#include "Anim_EnvironmentalAnimationController.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogEnvironmentalAnimation, Log, All);

UAnim_EnvironmentalAnimationController::UAnim_EnvironmentalAnimationController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS para performance
    
    // Configurações padrão para vegetação
    WindStrength = 1.0f;
    WindDirection = FVector(1.0f, 0.0f, 0.0f);
    WindFrequency = 0.5f;
    SwayAmplitude = 5.0f;
    
    // Configurações para água
    WaterWaveHeight = 10.0f;
    WaterWaveSpeed = 1.0f;
    WaterWaveFrequency = 2.0f;
    
    // Configurações para folhagem
    FoliageSwayIntensity = 1.0f;
    FoliageWindResponse = 0.8f;
    
    // Estados
    bIsWindActive = true;
    bIsWaterAnimationActive = true;
    bIsFoliageAnimationActive = true;
    
    // Timers
    WindVariationTimer = 0.0f;
    WaterAnimationTimer = 0.0f;
    
    // Cache
    CachedWindVector = FVector::ZeroVector;
    LastWindUpdate = 0.0f;
}

void UAnim_EnvironmentalAnimationController::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogEnvironmentalAnimation, Log, TEXT("Environmental Animation Controller iniciado"));
    
    // Inicializar componentes ambientais
    InitializeEnvironmentalComponents();
    
    // Configurar timer para variação de vento
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            WindVariationTimerHandle,
            this,
            &UAnim_EnvironmentalAnimationController::UpdateWindVariation,
            2.0f, // A cada 2 segundos
            true
        );
    }
}

void UAnim_EnvironmentalAnimationController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsWindActive && !bIsWaterAnimationActive && !bIsFoliageAnimationActive)
    {
        return;
    }
    
    // Atualizar animações ambientais
    UpdateWindAnimation(DeltaTime);
    UpdateWaterAnimation(DeltaTime);
    UpdateFoliageAnimation(DeltaTime);
    
    // Atualizar timers
    WindVariationTimer += DeltaTime;
    WaterAnimationTimer += DeltaTime;
}

void UAnim_EnvironmentalAnimationController::InitializeEnvironmentalComponents()
{
    if (!GetOwner())
    {
        return;
    }
    
    // Encontrar componentes de mesh estático para animar
    TArray<UStaticMeshComponent*> MeshComponents;
    GetOwner()->GetComponents<UStaticMeshComponent>(MeshComponents);
    
    for (UStaticMeshComponent* MeshComp : MeshComponents)
    {
        if (MeshComp && MeshComp->GetStaticMesh())
        {
            FString MeshName = MeshComp->GetStaticMesh()->GetName().ToLower();
            
            // Classificar tipo de mesh baseado no nome
            if (MeshName.Contains(TEXT("tree")) || MeshName.Contains(TEXT("plant")) || 
                MeshName.Contains(TEXT("grass")) || MeshName.Contains(TEXT("foliage")))
            {
                FoliageComponents.Add(MeshComp);
                UE_LOG(LogEnvironmentalAnimation, Log, TEXT("Componente de folhagem adicionado: %s"), *MeshName);
            }
            else if (MeshName.Contains(TEXT("water")) || MeshName.Contains(TEXT("lake")) || 
                     MeshName.Contains(TEXT("river")) || MeshName.Contains(TEXT("ocean")))
            {
                WaterComponents.Add(MeshComp);
                UE_LOG(LogEnvironmentalAnimation, Log, TEXT("Componente de água adicionado: %s"), *MeshName);
            }
        }
    }
    
    UE_LOG(LogEnvironmentalAnimation, Log, TEXT("Inicialização completa - Folhagem: %d, Água: %d"), 
           FoliageComponents.Num(), WaterComponents.Num());
}

void UAnim_EnvironmentalAnimationController::UpdateWindAnimation(float DeltaTime)
{
    if (!bIsWindActive)
    {
        return;
    }
    
    // Calcular vento procedural
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    // Vento base com variação senoidal
    FVector BaseWind = WindDirection * WindStrength;
    float WindVariation = FMath::Sin(CurrentTime * WindFrequency) * 0.3f + 0.7f;
    
    // Adicionar turbulência
    float TurbulenceX = FMath::Sin(CurrentTime * 1.7f) * 0.2f;
    float TurbulenceY = FMath::Cos(CurrentTime * 2.3f) * 0.15f;
    
    CachedWindVector = BaseWind * WindVariation + FVector(TurbulenceX, TurbulenceY, 0.0f);
    
    // Aplicar vento à folhagem
    ApplyWindToFoliage(DeltaTime);
}

void UAnim_EnvironmentalAnimationController::UpdateWaterAnimation(float DeltaTime)
{
    if (!bIsWaterAnimationActive || WaterComponents.Num() == 0)
    {
        return;
    }
    
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    for (UStaticMeshComponent* WaterComp : WaterComponents)
    {
        if (!WaterComp)
        {
            continue;
        }
        
        // Animação de ondas
        FVector OriginalLocation = WaterComp->GetComponentLocation();
        float WaveOffset = FMath::Sin(CurrentTime * WaterWaveSpeed + OriginalLocation.X * 0.01f) * WaterWaveHeight;
        
        FVector NewLocation = OriginalLocation;
        NewLocation.Z += WaveOffset * 0.1f; // Movimento sutil
        
        WaterComp->SetWorldLocation(NewLocation);
        
        // Rotação sutil para simular movimento de água
        FRotator CurrentRotation = WaterComp->GetComponentRotation();
        float RotationOffset = FMath::Sin(CurrentTime * WaterWaveFrequency) * 0.5f;
        CurrentRotation.Yaw += RotationOffset * DeltaTime;
        
        WaterComp->SetWorldRotation(CurrentRotation);
    }
}

void UAnim_EnvironmentalAnimationController::UpdateFoliageAnimation(float DeltaTime)
{
    if (!bIsFoliageAnimationActive)
    {
        return;
    }
    
    ApplyWindToFoliage(DeltaTime);
}

void UAnim_EnvironmentalAnimationController::ApplyWindToFoliage(float DeltaTime)
{
    if (FoliageComponents.Num() == 0)
    {
        return;
    }
    
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    for (UStaticMeshComponent* FoliageComp : FoliageComponents)
    {
        if (!FoliageComp)
        {
            continue;
        }
        
        // Calcular balanço baseado no vento
        FVector ComponentLocation = FoliageComp->GetComponentLocation();
        
        // Vento com variação baseada na posição (para evitar movimento uniforme)
        float PositionVariation = FMath::Sin(ComponentLocation.X * 0.001f + ComponentLocation.Y * 0.001f);
        float SwayX = CachedWindVector.X * FoliageSwayIntensity * PositionVariation;
        float SwayY = CachedWindVector.Y * FoliageSwayIntensity * PositionVariation;
        
        // Aplicar balanço como rotação
        FRotator CurrentRotation = FoliageComp->GetComponentRotation();
        FRotator TargetRotation = CurrentRotation;
        
        TargetRotation.Pitch += SwayX * SwayAmplitude * DeltaTime;
        TargetRotation.Roll += SwayY * SwayAmplitude * DeltaTime;
        
        // Interpolar suavemente para evitar movimentos bruscos
        FRotator NewRotation = UKismetMathLibrary::RInterpTo(
            CurrentRotation,
            TargetRotation,
            DeltaTime,
            FoliageWindResponse
        );
        
        FoliageComp->SetWorldRotation(NewRotation);
    }
}

void UAnim_EnvironmentalAnimationController::UpdateWindVariation()
{
    if (!bIsWindActive)
    {
        return;
    }
    
    // Variar direção do vento gradualmente
    float RandomYaw = FMath::RandRange(-30.0f, 30.0f);
    FRotator WindRotation = WindDirection.Rotation();
    WindRotation.Yaw += RandomYaw;
    WindDirection = WindRotation.Vector();
    
    // Variar força do vento
    WindStrength = FMath::Clamp(WindStrength + FMath::RandRange(-0.2f, 0.2f), 0.1f, 2.0f);
    
    UE_LOG(LogEnvironmentalAnimation, VeryVerbose, TEXT("Vento atualizado - Direção: %s, Força: %.2f"), 
           *WindDirection.ToString(), WindStrength);
}

void UAnim_EnvironmentalAnimationController::SetWindParameters(float NewStrength, const FVector& NewDirection, float NewFrequency)
{
    WindStrength = FMath::Clamp(NewStrength, 0.0f, 5.0f);
    WindDirection = NewDirection.GetSafeNormal();
    WindFrequency = FMath::Clamp(NewFrequency, 0.1f, 3.0f);
    
    UE_LOG(LogEnvironmentalAnimation, Log, TEXT("Parâmetros de vento atualizados - Força: %.2f, Direção: %s, Frequência: %.2f"), 
           WindStrength, *WindDirection.ToString(), WindFrequency);
}

void UAnim_EnvironmentalAnimationController::SetWaterParameters(float NewWaveHeight, float NewWaveSpeed, float NewWaveFrequency)
{
    WaterWaveHeight = FMath::Clamp(NewWaveHeight, 0.0f, 50.0f);
    WaterWaveSpeed = FMath::Clamp(NewWaveSpeed, 0.1f, 5.0f);
    WaterWaveFrequency = FMath::Clamp(NewWaveFrequency, 0.1f, 10.0f);
    
    UE_LOG(LogEnvironmentalAnimation, Log, TEXT("Parâmetros de água atualizados - Altura: %.2f, Velocidade: %.2f, Frequência: %.2f"), 
           WaterWaveHeight, WaterWaveSpeed, WaterWaveFrequency);
}

void UAnim_EnvironmentalAnimationController::EnableWindAnimation(bool bEnable)
{
    bIsWindActive = bEnable;
    UE_LOG(LogEnvironmentalAnimation, Log, TEXT("Animação de vento %s"), bEnable ? TEXT("ativada") : TEXT("desativada"));
}

void UAnim_EnvironmentalAnimationController::EnableWaterAnimation(bool bEnable)
{
    bIsWaterAnimationActive = bEnable;
    UE_LOG(LogEnvironmentalAnimation, Log, TEXT("Animação de água %s"), bEnable ? TEXT("ativada") : TEXT("desativada"));
}

void UAnim_EnvironmentalAnimationController::EnableFoliageAnimation(bool bEnable)
{
    bIsFoliageAnimationActive = bEnable;
    UE_LOG(LogEnvironmentalAnimation, Log, TEXT("Animação de folhagem %s"), bEnable ? TEXT("ativada") : TEXT("desativada"));
}

void UAnim_EnvironmentalAnimationController::RefreshEnvironmentalComponents()
{
    // Limpar arrays existentes
    FoliageComponents.Empty();
    WaterComponents.Empty();
    
    // Reinicializar
    InitializeEnvironmentalComponents();
    
    UE_LOG(LogEnvironmentalAnimation, Log, TEXT("Componentes ambientais atualizados"));
}

float UAnim_EnvironmentalAnimationController::GetCurrentWindStrength() const
{
    return WindStrength;
}

FVector UAnim_EnvironmentalAnimationController::GetCurrentWindDirection() const
{
    return WindDirection;
}

FVector UAnim_EnvironmentalAnimationController::GetCurrentWindVector() const
{
    return CachedWindVector;
}

int32 UAnim_EnvironmentalAnimationController::GetFoliageComponentCount() const
{
    return FoliageComponents.Num();
}

int32 UAnim_EnvironmentalAnimationController::GetWaterComponentCount() const
{
    return WaterComponents.Num();
}